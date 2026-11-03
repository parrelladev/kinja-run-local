package local.kinja;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;

/** Runs the production portable codec and transaction on synthetic host files only. */
public final class BackupStoreTest {
    static int assertions, scenarios;
    static Path work, assets;
    static void check(boolean value,String reason){assertions++;if(!value)throw new AssertionError(reason);}
    static final class Crash extends Error {}
    static final class Disk implements BackupStore.Disk {
        int operations, failAt; boolean crash;
        Disk(int failAt,boolean crash){this.failAt=failAt;this.crash=crash;}
        public void point(String name)throws IOException{if(++operations==failAt){if(crash)throw new Crash();throw new IOException("injected "+name);}}
        public void syncDirectory(File dir)throws IOException{point("sync:"+dir.getName());} // actual directory fsync belongs to Android integration
        public void rename(File from,File to)throws IOException{Files.move(from.toPath(),to.toPath(),StandardCopyOption.ATOMIC_MOVE,StandardCopyOption.REPLACE_EXISTING);}
    }
    static String prefs(int value){
        StringBuilder b=new StringBuilder("{\"Values\":[");
        List<String> keys=new ArrayList<>(Arrays.asList("1fight_fail_count","1drop_die_hint","boxguide_close_1","_manga_state"));
        for(int region:BackupCatalog.REGIONS)keys.add("fail_region_"+region);
        for(String key:keys){if(b.length()>11)b.append(',');b.append("{\"Key\":\"").append(key).append("\",\"Present\":true,\"Value\":").append(value).append('}');}
        return b.append("]}").toString();
    }
    static Map<String,byte[]> fixture(long gold)throws Exception{
        Map<String,byte[]> files=new LinkedHashMap<>();
        for(int i=0;i<8;i++)files.put(BackupStore.FILES[i],Files.readAllBytes(assets.resolve(BackupStore.FILES[i])));
        String player=new String(files.get("seed_player.json"),StandardCharsets.UTF_8).replace("1500",Long.toString(gold));
        files.put("seed_player.json",player.getBytes(StandardCharsets.UTF_8));
        files.put("seed_items.json","{\"ItemsInfo\":[{\"ItemId\":6001,\"Count\":224}]}".getBytes(StandardCharsets.UTF_8));
        files.put("seed_equipment.json","{\"EquipsInfo\":[{\"Uid\":3,\"EquipId\":3002,\"Level\":5,\"Quality\":2,\"Part\":3,\"IsWear\":true}]}".getBytes(StandardCharsets.UTF_8));
        files.put("hangup_local_state.json","{\"LastClaim\":1700000000,\"QuickReturnDay\":19675,\"QuickReturnCount\":2}".getBytes(StandardCharsets.UTF_8));
        files.put("shop_daily_claim.json","{\"Date\":20261103,\"Claimed\":true,\"Amount\":4000}".getBytes(StandardCharsets.UTF_8));
        files.put("local_preferences.json",prefs(gold==1500?0:1).getBytes(StandardCharsets.UTF_8));
        BackupStore.validate(files);return files;
    }
    static File root(String name,Map<String,byte[]> state)throws Exception{
        Path dir=Files.createTempDirectory(work,name);Path save=Files.createDirectory(dir.resolve("offline_save"));
        for(Map.Entry<String,byte[]> e:state.entrySet())if(!e.getKey().equals("local_preferences.json"))Files.write(save.resolve(e.getKey()),e.getValue());
        Files.write(save.resolve("qa_max_profile_applied.json"),"{\"applied\":true}".getBytes(StandardCharsets.UTF_8));
        return dir.toFile();
    }
    static void equalSave(File root,Map<String,byte[]> expected)throws Exception{
        for(String name:BackupStore.FILES){if(name.equals("local_preferences.json"))continue;
            File f=new File(new File(root,"offline_save"),name);
            check(expected.containsKey(name)==f.exists(),"presence "+name);
            if(expected.containsKey(name))check(Arrays.equals(expected.get(name),BackupStore.read(f)),"save "+name);
        }
    }
    interface Checked {void run()throws Exception;}
    static void rejects(Checked f,String reason)throws Exception{boolean failed=false;try{f.run();}catch(IOException e){failed=true;}check(failed,reason);}
    static byte[] rehash(byte[] bytes)throws Exception{byte[] body=Arrays.copyOf(bytes,bytes.length-32),h=BackupStore.hash(body);System.arraycopy(h,0,bytes,body.length,32);return bytes;}
    static Map<String,byte[]> copy(Map<String,byte[]> files){return new LinkedHashMap<>(files);}
    public static void main(String[] args)throws Exception{
        assets=Paths.get(args[0]);work=Paths.get(args[1]);Files.createDirectories(work);
        Map<String,byte[]> old=fixture(1500),incoming=fixture(123456);byte[] encoded=BackupStore.encode(incoming);
        final byte[][] output={null};
        BackupStore.export(encoded,new BackupStore.ExportFile(){public void writeAndSync(byte[] b){output[0]=b.clone();}public byte[] readBack(){return output[0];}});
        check(Arrays.equals(output[0],encoded),"export complete verified");
        rejects(()->BackupStore.export(encoded,new BackupStore.ExportFile(){public void writeAndSync(byte[] b)throws IOException{output[0]=Arrays.copyOf(b,20);throw new IOException("disk full");}public byte[] readBack(){throw new AssertionError("must not read/succeed after failed write");}}),"partial export write failure");
        rejects(()->BackupStore.export(encoded,new BackupStore.ExportFile(){public void writeAndSync(byte[] b){}public byte[] readBack(){return new byte[]{1,2};}}),"export readback mismatch");
        rejects(()->BackupStore.export(encoded,new BackupStore.ExportFile(){public void writeAndSync(byte[] b){}public byte[] readBack()throws IOException{throw new IOException("read permission denied");}}),"export readback denied");
        check(BackupStore.decode(encoded).keySet().equals(incoming.keySet()),"portable round trip names");
        for(String name:incoming.keySet())check(Arrays.equals(BackupStore.decode(encoded).get(name),incoming.get(name)),"portable bytes "+name);
        byte[] damaged=encoded.clone();damaged[50]^=1;rejects(()->BackupStore.decode(damaged),"bad checksum");
        rejects(()->BackupStore.decode(Arrays.copyOf(encoded,40)),"truncated");
        byte[] future=encoded.clone();future[11]=2;rehash(future);rejects(()->BackupStore.decode(future),"future format with valid digest");
        byte[] wrongClient=encoded.clone();wrongClient[16]^=1;rehash(wrongClient);rejects(()->BackupStore.decode(wrongClient),"different client");
        rejects(()->BackupStore.read(new ByteArrayInputStream(new byte[200]),100),"bounded provider read");
        Map<String,byte[]> missing=copy(incoming);missing.remove("seed_heroes.json");rejects(()->BackupStore.encode(missing),"missing required model");
        Map<String,byte[]> unknown=copy(incoming);unknown.put("../private.json",new byte[]{1,2});rejects(()->BackupStore.encode(unknown),"path traversal/unknown entry");
        for(String bad:new String[]{"{\"ItemsInfo\":[],\"ItemsInfo\":[]}","{\"ItemsInfo\":null}","{\"ItemsInfo\":[{\"ItemId\":6001,\"Count\":-1}]}","{\"ItemsInfo\":[{\"ItemId\":2147483647,\"Count\":1}]}","{\"ItemsInfo\":[]} trailing"}){
            Map<String,byte[]> malformed=copy(incoming);malformed.put("seed_items.json",bad.getBytes(StandardCharsets.UTF_8));rejects(()->BackupStore.encode(malformed),"strict schema/json");
        }
        File normal=root("success-",old);BackupStore store=new BackupStore(normal,new Disk(0,false));
        File alias=new File(normal,"../"+normal.getName());
        check(BackupStore.snapshot(new File(alias,"offline_save"),prefs(0)).size()==old.size(),"canonical trusted parent snapshot");
        check(new BackupStore(alias,new Disk(0,false)).boot()==null,"canonical trusted parent boot");
        store.request(encoded);equalSave(normal,old);check(store.boot()==null,"no premature success");equalSave(normal,incoming);
        check(store.preferences(prefs(0)).contains("\"Value\":1"),"incoming logical preference checkpoint");
        check(new File(normal,"manual-import.previous/local_preferences.json").exists(),"recovery prefs saved first");
        check(store.initialized(),"original callbacks ack");equalSave(normal,incoming);check(store.boot()==null,"idempotent restart");
        check(new File(normal,"offline_save/qa_max_profile_applied.json").exists(),"destination QA marker preserved");
        check(!BackupStore.decode(encoded).containsKey("qa_max_profile_applied.json"),"QA not portable");
        // Each failure point during staging/swap/preferences/commit, with both IOException and process death.
        File baseline=root("count-",old);Disk count=new Disk(0,false);BackupStore counted=new BackupStore(baseline,count);counted.request(encoded);counted.boot();counted.preferences(prefs(0));counted.initialized();int total=count.operations;
        for(boolean processDeath:new boolean[]{false,true})for(int point=1;point<=total;point++){
            File dir=root("fault-",old);Disk fault=new Disk(point,processDeath);BackupStore target=new BackupStore(dir,fault);
            try{target.request(encoded);target.boot();target.preferences(prefs(0));target.initialized();}catch(IOException|Crash expected){}
            BackupStore reopened=new BackupStore(dir,new Disk(0,false));
            String notice=reopened.boot();
            // A request durable before a failure may be applied on the next launch. Ack it now.
            if(new File(dir,"manual-import.state").exists()){
                reopened.preferences(prefs(0));reopened.initialized();equalSave(dir,incoming);
            }else{
                String player=new String(BackupStore.read(new File(dir,"offline_save/seed_player.json")),StandardCharsets.UTF_8);
                if(player.contains("123456"))equalSave(dir,incoming);else equalSave(dir,old);
            }
            scenarios++;check(new File(dir,"offline_save/qa_max_profile_applied.json").exists(),"QA after interruption");
        }
        File rollback=root("rollback-",old);BackupStore interrupted=new BackupStore(rollback,new Disk(0,false));interrupted.request(encoded);interrupted.boot();interrupted.preferences(prefs(0));
        check(new BackupStore(rollback,new Disk(0,false)).boot().contains("interrompida"),"unacknowledged bootstrap rolls back");equalSave(rollback,old);
        String checkpoint=new BackupStore(rollback,new Disk(0,false)).preferences(prefs(1));check(checkpoint.contains("\"Value\":0"),"logical prefs restored after interrupted import");
        for(String journal:BackupStore.JOURNALS){File dir=root("journal-",old);Files.write(new File(dir,"offline_save/"+journal).toPath(),new byte[]{1});rejects(()->BackupStore.snapshot(new File(dir,"offline_save"),prefs(0)),"pending journal "+journal);equalSave(dir,old);}
        Map<String,byte[]> noOptional=copy(incoming);noOptional.remove("hangup_local_state.json");noOptional.remove("shop_daily_claim.json");
        File optional=root("optional-",old);BackupStore optionalStore=new BackupStore(optional,new Disk(0,false));optionalStore.request(BackupStore.encode(noOptional));optionalStore.boot();optionalStore.preferences(prefs(0));optionalStore.initialized();equalSave(optional,noOptional);
        System.out.println("PASS assertions="+assertions+" faultScenarios="+scenarios+" faultPoints="+total+"; production codec/transaction on host fixtures only; Android fsync/SAF/Unity untested.");
    }
}
