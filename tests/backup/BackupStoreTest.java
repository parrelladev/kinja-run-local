package local.kinja;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;

/** Runs production codec/transaction on synthetic files, on host or explicitly opted-in ART. */
public final class BackupStoreTest {
    static int assertions, scenarios;
    static boolean android;
    static Path work, assets;
    static void check(boolean value,String reason){assertions++;if(!value)throw new AssertionError(reason);}
    static final class Crash extends Error {}
    static final class Disk implements BackupStore.Disk {
        int operations, failAt; boolean crash;
        Disk(int failAt,boolean crash){this.failAt=failAt;this.crash=crash;}
        public void point(String name)throws IOException{if(++operations==failAt){if(crash)throw new Crash();throw new IOException("injected "+name);}}
        public void syncDirectory(File dir)throws IOException{
            point("sync:"+dir.getName());
            if(!android)return; // Host does not pretend to exercise Android directory fsync.
            FileDescriptor fd=null;
            try {
                Class<?> os=Class.forName("android.system.Os"), constants=Class.forName("android.system.OsConstants");
                int readOnly=constants.getField("O_RDONLY").getInt(null);
                fd=(FileDescriptor)os.getMethod("open",String.class,int.class,int.class).invoke(null,dir.getAbsolutePath(),readOnly,0);
                os.getMethod("fsync",FileDescriptor.class).invoke(null,fd);
            } catch(Exception e){throw new IOException("Real Android directory fsync failed",e);}
            finally {if(fd!=null)try{Class.forName("android.system.Os").getMethod("close",FileDescriptor.class).invoke(null,fd);}catch(Exception ignored){}}
        }
        public void rename(File from,File to)throws IOException{
            if(!android){Files.move(from.toPath(),to.toPath(),StandardCopyOption.ATOMIC_MOVE,StandardCopyOption.REPLACE_EXISTING);return;}
            try {Class.forName("android.system.Os").getMethod("rename",String.class,String.class).invoke(null,from.getAbsolutePath(),to.getAbsolutePath());}
            catch(Exception e){throw new IOException("Real Android rename failed",e);}
        }
    }
    static String prefs(int value){
        StringBuilder b=new StringBuilder("{\"Values\":[");
        List<String> keys=new ArrayList<>(Arrays.asList("1fight_fail_count","1drop_die_hint","boxguide_close_1","_manga_state"));
        for(int region:BackupCatalog.REGIONS)keys.add("fail_region_"+region);
        for(String key:keys){if(b.length()>11)b.append(',');b.append("{\"Key\":\"").append(key).append("\",\"Present\":true,\"Value\":").append(value).append('}');}
        b.append(",{\"Key\":\"game_language\",\"Present\":true,\"Value\":").append(value==0?2:8).append('}');
        for(String key:new String[]{"sound_volume","music_volume","operation_sensitivity"}) {
            float setting=key.equals("operation_sensitivity")?(value==0?0.33f:0.75f):(value==0?1f:0f);
            b.append(",{\"Key\":\"").append(key).append("\",\"Present\":true,\"Type\":\"float\",\"Value\":")
                .append(Float.floatToRawIntBits(setting)&0xffffffffL).append('}');
        }
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
        android=args.length>2 && args[2].equals("android");
        if(args.length>2 && !android)throw new IOException("Unknown fixture runtime mode");
        assets=Paths.get(args[0]);work=Paths.get(args[1]);
        if(android){
            String scope=work.toFile().getCanonicalFile().getParent();
            if(scope==null || !scope.matches("/data/local/tmp/kinja-backup-qa-[a-zA-Z0-9-]+") ||
                    !work.toFile().getCanonicalPath().equals(scope+"/fixtures") ||
                    !assets.toFile().getCanonicalPath().equals(scope+"/assets"))
                throw new IOException("ART fixtures must stay in the unique synthetic QA sandbox");
        }
        Files.createDirectories(work);
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
        byte[] future=encoded.clone();future[11]=3;rehash(future);rejects(()->BackupStore.decode(future),"future format with valid digest");
        byte[] legacy=encoded.clone();legacy[11]=1;rehash(legacy);rejects(()->BackupStore.decode(legacy),"incomplete legacy format explicitly incompatible");
        byte[] wrongClient=encoded.clone();wrongClient[16]^=1;rehash(wrongClient);rejects(()->BackupStore.decode(wrongClient),"different client");
        rejects(()->BackupStore.read(new ByteArrayInputStream(new byte[200]),100),"bounded provider read");
        Map<String,byte[]> missing=copy(incoming);missing.remove("seed_heroes.json");rejects(()->BackupStore.encode(missing),"missing required model");
        Map<String,byte[]> unknown=copy(incoming);unknown.put("../private.json",new byte[]{1,2});rejects(()->BackupStore.encode(unknown),"path traversal/unknown entry");
        for(String bad:new String[]{"{\"ItemsInfo\":[],\"ItemsInfo\":[]}","{\"ItemsInfo\":null}","{\"ItemsInfo\":[{\"ItemId\":6001,\"Count\":-1}]}","{\"ItemsInfo\":[{\"ItemId\":2147483647,\"Count\":1}]}","{\"ItemsInfo\":[]} trailing"}){
            Map<String,byte[]> malformed=copy(incoming);malformed.put("seed_items.json",bad.getBytes(StandardCharsets.UTF_8));rejects(()->BackupStore.encode(malformed),"strict schema/json");
        }
        String settings=prefs(1);
        String sound="\"Key\":\"sound_volume\",\"Present\":true,\"Type\":\"float\",\"Value\":0";
        for(long bits:new long[]{0x7fc00000L,0x7f800000L,0xff800000L,0xbf000000L,0x40000000L,0x100000000L}) {
            Map<String,byte[]> malformed=copy(incoming);
            malformed.put("local_preferences.json",settings.replace(sound,"\"Key\":\"sound_volume\",\"Present\":true,\"Type\":\"float\",\"Value\":"+bits).getBytes(StandardCharsets.UTF_8));
            rejects(()->BackupStore.encode(malformed),"invalid float bits "+bits);
        }
        for(String changed:new String[]{settings.replace("\"Type\":\"float\"","\"Type\":\"int\""),
                settings.replace(",\"Type\":\"float\"",""),
                settings.replace(",{\"Key\":\"sound_volume\",\"Present\":true,\"Type\":\"float\",\"Value\":0}",""),
                settings.replace("\"Key\":\"game_language\",\"Present\":true,\"Value\":8","\"Key\":\"game_language\",\"Present\":true,\"Value\":16"),
                settings.replace(sound,"\"Key\":\"sound_volume\",\"Present\":false,\"Type\":\"float\",\"Value\":0")}) {
            Map<String,byte[]> malformed=copy(incoming);malformed.put("local_preferences.json",changed.getBytes(StandardCharsets.UTF_8));
            rejects(()->BackupStore.encode(malformed),"typed settings schema rejects mismatch");
        }
        Map<String,byte[]> absentSettings=copy(old);
        String absent=prefs(0).replace("\"Key\":\"game_language\",\"Present\":true,\"Value\":2","\"Key\":\"game_language\",\"Present\":false,\"Value\":8");
        for(String key:new String[]{"sound_volume","music_volume","operation_sensitivity"})
            absent=absent.replace("\"Key\":\""+key+"\",\"Present\":true","\"Key\":\""+key+"\",\"Present\":false");
        absentSettings.put("local_preferences.json",absent.getBytes(StandardCharsets.UTF_8));
        BackupStore.encode(absentSettings);check(true,"absent float defaults and effective system language accepted");
        Map<String,byte[]> fractional=copy(incoming);
        String exact=settings.replace(sound,"\"Key\":\"sound_volume\",\"Present\":true,\"Type\":\"float\",\"Value\":"+(Float.floatToRawIntBits(0.1234567f)&0xffffffffL));
        fractional.put("local_preferences.json",exact.getBytes(StandardCharsets.UTF_8));
        check(Arrays.equals(BackupStore.decode(BackupStore.encode(fractional)).get("local_preferences.json"),fractional.get("local_preferences.json")),"fractional configuration binary32 bits preserved exactly");
        File normal=root("success-",old);BackupStore store=new BackupStore(normal,new Disk(0,false));
        File alias=new File(normal,"../"+normal.getName());
        check(BackupStore.snapshot(new File(alias,"offline_save"),prefs(0)).size()==old.size(),"canonical trusted parent snapshot");
        check(new BackupStore(alias,new Disk(0,false)).boot()==null,"canonical trusted parent boot");
        store.request(encoded);equalSave(normal,old);check(store.boot()==null,"no premature success");equalSave(normal,incoming);
        check(store.preferences(prefs(0)).contains("\"Value\":1"),"incoming logical preference checkpoint");
        String typedCheckpoint=store.preferences(prefs(1)); // same-process retry after imported/mixed prefs became visible
        check(Arrays.equals(BackupStore.decode(BackupStore.read(new File(normal,"manual-import.recovery"))).get("local_preferences.json"),old.get("local_preferences.json")),"same-process retry preserves original recovery preferences");
        check(Arrays.equals(BackupStore.read(new File(normal,"manual-import.previous/local_preferences.json")),old.get("local_preferences.json")),"same-process retry preserves original recovery checkpoint");
        check(typedCheckpoint.contains("\"Type\":\"float\""),"checkpoint retains original float type");
        check(typedCheckpoint.contains(Long.toString(Float.floatToRawIntBits(0.75f)&0xffffffffL)),"checkpoint retains exact sensitivity bits");
        check(typedCheckpoint.contains("\"Key\":\"game_language\",\"Present\":true,\"Value\":8"),"checkpoint retains Portuguese selection");
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
        interrupted.preferences(prefs(1));
        check(new BackupStore(rollback,new Disk(0,false)).boot().contains("interrompida"),"unacknowledged bootstrap rolls back");equalSave(rollback,old);
        String checkpoint=new BackupStore(rollback,new Disk(0,false)).preferences(prefs(1));check(checkpoint.equals(prefs(0)),"all original logical preferences restored after interrupted import and polluted same-process retry");
        File corrupt=root("checkpoint-corrupt-",old);BackupStore corruptStore=new BackupStore(corrupt,new Disk(0,false));
        corruptStore.request(encoded);corruptStore.boot();corruptStore.preferences(prefs(0));
        Files.write(new File(corrupt,"manual-import.previous/local_preferences.json").toPath(),prefs(1).getBytes(StandardCharsets.UTF_8));
        rejects(()->corruptStore.preferences(prefs(1)),"conflicting durable recovery checkpoint fails closed");
        check(Arrays.equals(BackupStore.decode(BackupStore.read(new File(corrupt,"manual-import.recovery"))).get("local_preferences.json"),old.get("local_preferences.json")),"conflicting checkpoint never overwrites valid recovery");
        for(String journal:BackupStore.JOURNALS){File dir=root("journal-",old);Files.write(new File(dir,"offline_save/"+journal).toPath(),new byte[]{1});rejects(()->BackupStore.snapshot(new File(dir,"offline_save"),prefs(0)),"pending journal "+journal);equalSave(dir,old);}
        Map<String,byte[]> noOptional=copy(incoming);noOptional.remove("hangup_local_state.json");noOptional.remove("shop_daily_claim.json");
        File optional=root("optional-",old);BackupStore optionalStore=new BackupStore(optional,new Disk(0,false));optionalStore.request(BackupStore.encode(noOptional));optionalStore.boot();optionalStore.preferences(prefs(0));optionalStore.initialized();equalSave(optional,noOptional);
        System.out.println("PASS assertions="+assertions+" faultScenarios="+scenarios+" faultPoints="+total+
            (android?"; production codec/transaction on synthetic ART fixtures; real Android file/directory fsync and Os.rename; injected exceptions/crashes only, no OS kill/power loss; SAF/Unity/private game saves untested.":
            "; production codec/transaction on host fixtures only; Android fsync/SAF/Unity untested."));
    }
}
