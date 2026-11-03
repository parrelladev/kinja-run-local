package local.kinja;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.*;

/** Portable format and crash recovery. No Android or Unity dependencies. */
final class BackupStore {
    static final int LIMIT = 128 * 1024;
    static final String CLIENT = "kinja-1.9.1-7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e/offline-save-1";
    static final String[] FILES = {"seed_player.json", "seed_regions.json", "seed_items.json",
        "seed_equipment.json", "seed_talents.json", "seed_heroes.json", "seed_guide.json",
        "seed_shop_chests.json", "hangup_local_state.json", "shop_daily_claim.json", "local_preferences.json"};
    static final String[] JOURNALS = {"talent_purchase_pending.json", "equip_upgrade_pending",
        "chest_purchase_pending", "hero_pending", "shop_daily_claim_pending.json"};
    interface Disk {
        void syncDirectory(File dir) throws IOException;
        void rename(File from, File to) throws IOException;
        void point(String name) throws IOException;
    }
    interface ExportFile {
        void writeAndSync(byte[] bytes) throws IOException;
        byte[] readBack() throws IOException;
    }
    static void export(byte[] backup, ExportFile target) throws IOException {
        decode(backup);
        target.writeAndSync(backup);
        byte[] written=target.readBack();
        if (!Arrays.equals(backup,written))throw new IOException("O arquivo gravado não passou na conferência.");
        decode(written);
    }
    private final File root;
    private final Disk disk;
    BackupStore(File root, Disk disk) throws IOException { this.root = root.getCanonicalFile(); this.disk = disk; }
    private File at(String name) { return new File(root, name); }
    static IOException invalid() { return new IOException("Backup inválido, incompleto ou incompatível."); }
    static byte[] read(InputStream in, int limit) throws IOException {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        byte[] buffer = new byte[8192]; int n;
        while ((n = in.read(buffer)) != -1) {
            if (out.size() + n > limit) throw invalid();
            out.write(buffer, 0, n);
        }
        return out.toByteArray();
    }
    static byte[] read(File file) throws IOException {
        try (InputStream in = new FileInputStream(file)) { return read(in, LIMIT * 12); }
    }
    static byte[] hash(byte[] data) throws IOException {
        try { return MessageDigest.getInstance("SHA-256").digest(data); }
        catch (Exception e) { throw new IOException("SHA-256 indisponível.", e); }
    }
    static byte[] encode(Map<String, byte[]> files) throws IOException {
        validate(files);
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        DataOutputStream out = new DataOutputStream(buffer);
        out.writeLong(0x4b494e4a41424b50L); // KINJABKP
        out.writeInt(1); out.writeUTF(CLIENT); out.writeInt(files.size());
        for (String name : FILES) if (files.containsKey(name)) {
            byte[] data = files.get(name);
            out.writeUTF(name); out.writeInt(data.length); out.write(data);
        }
        out.flush(); byte[] body = buffer.toByteArray();
        out.write(hash(body)); out.flush(); return buffer.toByteArray();
    }
    static Map<String, byte[]> decode(byte[] data) throws IOException {
        if (data.length < 48 || data.length > LIMIT * 12) throw invalid();
        byte[] body = Arrays.copyOf(data, data.length - 32);
        if (!MessageDigest.isEqual(hash(body), Arrays.copyOfRange(data, body.length, data.length))) throw invalid();
        DataInputStream in = new DataInputStream(new ByteArrayInputStream(body));
        if (in.readLong() != 0x4b494e4a41424b50L) throw invalid();
        if (in.readInt() != 1 || !CLIENT.equals(in.readUTF()))
            throw new IOException("Versão de backup incompatível com este Kinja Run offline 1.9.1.");
        int count = in.readInt(); if (count < 9 || count > FILES.length) throw invalid();
        Map<String, byte[]> files = new LinkedHashMap<>();
        for (int i = 0; i < count; i++) {
            String name = in.readUTF(); int size = in.readInt();
            if (!Arrays.asList(FILES).contains(name) || files.containsKey(name) || size < 2 || size > LIMIT) throw invalid();
            byte[] bytes = new byte[size]; in.readFully(bytes); files.put(name, bytes);
        }
        if (in.available() != 0) throw invalid();
        validate(files); return files;
    }
    static Map<String, byte[]> snapshot(File dir, String preferences) throws IOException {
        // Android filesDir may legitimately traverse /data/user/0 -> /data/data.
        // Resolve the trusted parent once; still reject linked child save files.
        dir = dir.getCanonicalFile();
        for (String journal : JOURNALS) if (new File(dir, journal).exists())
            throw new IOException("Há uma operação de progresso pendente. Feche e reabra o jogo antes do backup.");
        Map<String, byte[]> files = new LinkedHashMap<>();
        for (String name : FILES) {
            if (name.equals("local_preferences.json")) continue;
            File file = new File(dir, name);
            if (file.exists()) {
                if (!file.isFile() || !file.getCanonicalFile().equals(file.getAbsoluteFile())) throw invalid();
                byte[] data = read(file); if (data.length > LIMIT) throw invalid(); files.put(name, data);
            }
        }
        files.put("local_preferences.json", preferences.getBytes(StandardCharsets.UTF_8));
        validate(files); return files;
    }
    private void write(File file, byte[] data) throws IOException {
        disk.point("write:" + file.getName());
        try (FileOutputStream out = new FileOutputStream(file)) {
            out.write(data); out.flush(); out.getFD().sync();
        }
        if (!Arrays.equals(read(file), data)) throw new IOException("Falha ao conferir o arquivo gravado.");
    }
    private void atomic(File file, byte[] data) throws IOException {
        File temp = new File(file.getPath() + ".tmp"); write(temp, data);
        move(temp, file); disk.syncDirectory(file.getParentFile());
    }
    private void move(File from, File to) throws IOException {
        disk.point("rename:" + from.getName());
        disk.rename(from,to);
    }
    private void remove(File file) throws IOException {
        if (!file.exists()) return;
        if (!file.getCanonicalFile().equals(file.getAbsoluteFile())) throw invalid();
        if (file.isDirectory()) {
            File[] children = file.listFiles(); if (children == null) throw invalid();
            for (File child : children) remove(child);
        }
        if (!file.delete()) throw new IOException("Não foi possível limpar uma etapa de recuperação.");
    }
    /** Queue only; the live save is never changed in the running game. */
    void request(byte[] backup) throws IOException {
        decode(backup);
        if (at("manual-import.state").exists() || at("manual-import.request").exists())
            throw new IOException("Já existe uma importação pendente. Reabra o jogo.");
        atomic(at("manual-import.request"), backup);
    }
    private void state(String value) throws IOException {
        atomic(at("manual-import.state"), value.getBytes(StandardCharsets.UTF_8));
    }
    private String phase() throws IOException {
        return at("manual-import.state").exists() ? new String(read(at("manual-import.state")), StandardCharsets.UTF_8) : "";
    }
    boolean applying() throws IOException { return phase().equals("applying"); }
    /** Runs before Unity and native configure. Any unacknowledged bootstrap rolls back. */
    String boot() throws IOException {
        String phase = phase();
        File save = at("offline_save"), previous = at("manual-import.previous"), next = at("manual-import.next");
        if (phase.equals("prepared") || phase.equals("applying")) {
            if (previous.exists()) {
                // Verify recovery bytes BEFORE removing the failed candidate.
                Map<String, byte[]> recovery = decode(read(at("manual-import.recovery")));
                verifyDirectory(previous, recovery, false);
                remove(at("manual-import.failed"));
                if (save.exists()) move(save, at("manual-import.failed"));
                move(previous, save); disk.syncDirectory(root);
            } else if (!save.exists()) throw new IOException("Recuperação necessária: progresso anterior não encontrado.");
            remove(next); remove(at("manual-import.request")); remove(at("manual-import.state")); disk.syncDirectory(root);
            return "A importação foi interrompida. O progresso anterior foi preservado; nenhuma importação foi confirmada.";
        }
        if (phase.equals("committed")) {
            remove(at("manual-import.request")); remove(at("manual-import.state")); disk.syncDirectory(root);
            return "Backup importado e progresso carregado corretamente.";
        }
        if (!phase.isEmpty()) throw invalid();
        if (!at("manual-import.request").exists()) return null;
        Map<String, byte[]> incoming = decode(read(at("manual-import.request")));
        // A second validation occurs against the immutable request, on this cold start.
        String oldPrefs = new String(incoming.get("local_preferences.json"), StandardCharsets.UTF_8); // placeholder; never applied, replaced before any preference mutation
        Map<String, byte[]> current = snapshot(save, oldPrefs);
        atomic(at("manual-import.recovery"), encode(current));
        remove(next); remove(previous); remove(at("manual-import.failed"));
        if (!next.mkdir()) throw new IOException("Sem espaço ou permissão para preparar a importação.");
        for (Map.Entry<String, byte[]> entry : incoming.entrySet()) write(new File(next, entry.getKey()), entry.getValue());
        // Keep destination-only QA idempotency markers. Never transfer them from the portable file.
        File[] markers = save.listFiles(); if (markers == null) throw invalid();
        for (File marker : markers) if (marker.isFile() && marker.getName().startsWith("qa_") && marker.getName().endsWith(".json"))
            write(new File(next, marker.getName()), read(marker));
        verifyDirectory(next, incoming, true); disk.syncDirectory(next);
        state("prepared"); move(save, previous); disk.syncDirectory(root);
        move(next, save); disk.syncDirectory(root); state("applying");
        return null; // Success is reported only after original Unity callbacks acknowledge the import.
    }
    private void verifyDirectory(File dir, Map<String, byte[]> files, boolean preferences) throws IOException {
        for (Map.Entry<String, byte[]> entry : files.entrySet()) {
            if (!preferences && entry.getKey().equals("local_preferences.json")) continue;
            if (!Arrays.equals(read(new File(dir, entry.getKey())), entry.getValue())) throw invalid();
        }
    }
    /** Called on Unity thread before applying the incoming logical preferences. */
    void saveRecoveryPreferences(String json) throws IOException {
        if (!phase().equals("applying")) return;
        Map<String, byte[]> old = decode(read(at("manual-import.recovery")));
        old.put("local_preferences.json", json.getBytes(StandardCharsets.UTF_8)); validate(old);
        atomic(at("manual-import.recovery"), encode(old));
        atomic(new File(at("manual-import.previous"), "local_preferences.json"), old.get("local_preferences.json"));
    }
    String preferences(String current) throws IOException {
        File checkpoint = new File(at("offline_save"), "local_preferences.json");
        if (!checkpoint.exists()) return "";
        // Save recovery before any original Set/Delete call, including a crash partway through preferences.
        saveRecoveryPreferences(current);
        Map<String,Object> model=obj(new Json(read(checkpoint)).parse());
        StringBuilder out=new StringBuilder("{\"Values\":[");
        for(Object entry:list(model,"Values")) {
            Map<String,Object> v=obj(entry);
            if(out.length()>11)out.append(',');
            out.append("{\"Key\":\"").append(v.get("Key")).append("\",\"Present\":").append(bool(v,"Present"))
                .append(",\"Value\":").append(num(v,"Value",0,Integer.MAX_VALUE)).append('}');
        }
        return out.append("]}").toString();
    }
    boolean initialized() throws IOException {
        boolean imported = phase().equals("applying");
        // Checkpoint is replayable until all original initialization callbacks succeed.
        remove(new File(at("offline_save"), "local_preferences.json")); disk.syncDirectory(at("offline_save"));
        if (imported) { state("committed"); remove(at("manual-import.request")); remove(at("manual-import.state")); disk.syncDirectory(root); }
        return imported;
    }

    @SuppressWarnings("unchecked") static Map<String, Object> obj(Object value) throws IOException {
        if (!(value instanceof Map)) throw invalid(); return (Map<String, Object>)value;
    }
    @SuppressWarnings("unchecked") static List<Object> list(Map<String, Object> value, String key) throws IOException {
        Object result = value.get(key); if (!(result instanceof List)) throw invalid(); return (List<Object>)result;
    }
    static long num(Map<String, Object> value, String key, long min, long max) throws IOException {
        Object result = value.get(key); if (!(result instanceof Long)) throw invalid();
        long n = (Long)result; if (n < min || n > max) throw invalid(); return n;
    }
    static boolean bool(Map<String, Object> value, String key) throws IOException {
        Object result = value.get(key); if (!(result instanceof Boolean)) throw invalid(); return (Boolean)result;
    }
    static void ints(List<Object> values) throws IOException { for (Object n : values) if (!(n instanceof Long) || (Long)n < 0 || (Long)n > Integer.MAX_VALUE) throw invalid(); }
    static void validate(Map<String, byte[]> files) throws IOException {
        Map<String, Map<String, Object>> models = new HashMap<>();
        for (int i = 0; i < FILES.length; i++) {
            byte[] data = files.get(FILES[i]);
            if (data == null) { if (i < 8 || i == 10) throw invalid(); continue; }
            if (data.length < 2 || data.length > LIMIT) throw invalid();
            models.put(FILES[i], obj(new Json(data).parse()));
        }
        if (models.size() != files.size()) throw invalid();
        Map<String, Object> p = models.get(FILES[0]);
        num(p,"Level",1,200); for (String k : new String[]{"Coin","Diamond","Exp","Power","RenameCount","Icon","IconFrame","CumulativeLoginDays","Union","UnionHelpLastTime","RegTime","LastTime"}) num(p,k,0,Long.MAX_VALUE);
        for (String k : new String[]{"Coin","Diamond","Exp","Power"}) num(p,k,0,Integer.MAX_VALUE);
        if (!(p.get("PlayerName") instanceof String) || ((String)p.get("PlayerName")).length() > 256 || !(p.get("Lang") instanceof String)) throw invalid();
        bool(p,"IsNewDay"); List<Object> statuses = list(p,"PlayerStatus"), times = list(p,"PlayerStatusTime");
        if (statuses.size()!=times.size()) throw invalid();
        for (Object s : statuses) if (!(s instanceof String) || !((String)s).matches("[0-9]{1,10}")) throw invalid();
        for (Object t : times) if (!(t instanceof Long) || (Long)t<0) throw invalid();
        Map<String, Object> r = models.get(FILES[1]);
        long maximum=num(r,"MaxRegion",1,Integer.MAX_VALUE), selected=num(r,"SelectRegion",1,Integer.MAX_VALUE);
        num(r,"MaxRegionProgress",0,100000); ints(list(r,"SupportGifts"));
        Set<Long> regions = new HashSet<>();
        for (Object entry : list(r,"RegionsInfo")) {
            Map<String,Object> v=obj(entry); long id=num(v,"RegionId",1,Integer.MAX_VALUE);
            if (!BackupCatalog.has(BackupCatalog.REGIONS,id) || !regions.add(id)) throw invalid(); bool(v,"IsPass"); num(v,"MaxScore",0,Integer.MAX_VALUE); num(v,"ConsecutiveLostCount",0,Integer.MAX_VALUE);
            for(Object b:list(v,"DifficultPass")) if (!(b instanceof Boolean)) throw invalid();
        }
        if (!regions.contains(maximum) || !regions.contains(selected)) throw invalid();
        Set<Long> items = new HashSet<>();
        for(Object entry:list(models.get(FILES[2]),"ItemsInfo")) {
            Map<String,Object> v=obj(entry); long id=num(v,"ItemId",1,Integer.MAX_VALUE); num(v,"Count",0,Integer.MAX_VALUE);
            if (id==101 || id==102 || id==104 || !BackupCatalog.has(BackupCatalog.ITEMS,id) || !items.add(id)) throw invalid();
        }
        Set<Long> uids=new HashSet<>(), worn=new HashSet<>();
        for(Object entry:list(models.get(FILES[3]),"EquipsInfo")) {
            Map<String,Object> v=obj(entry); long uid=num(v,"Uid",1,Integer.MAX_VALUE), part=num(v,"Part",1,6);
            long equip=num(v,"EquipId",1,Integer.MAX_VALUE), level=num(v,v.containsKey("EquipLevel")?"EquipLevel":"Level",1,1000), quality=num(v,"Quality",1,20);
            if(v.containsKey("EquipLevel") && v.containsKey("Level") && num(v,"Level",1,1000)!=level)throw invalid();
            boolean found=false;for(int[] cfg:BackupCatalog.EQUIPS)if(cfg[0]==equip && cfg[1]==part)found=true;if(!found)throw invalid();
            found=false;for(int[] cfg:BackupCatalog.QUALITY)if(cfg[0]==quality && level<=cfg[1])found=true;if(!found)throw invalid();
            if (!uids.add(uid) || (bool(v,"IsWear") && !worn.add(part))) throw invalid();
        }
        Map<String,Object> t=models.get(FILES[4]); num(t,"TalentLevel",0,BackupCatalog.TALENT_MAX); num(t,"ExtraTalentLevel",0,BackupCatalog.EXTRA_MAX);
        Map<String,Object> h=models.get(FILES[5]); long equipped=num(h,"EquipHeroId",1,Integer.MAX_VALUE); Set<Long> heroes=new HashSet<>();
        for(Object entry:list(h,"HerosInfo")) { Map<String,Object> v=obj(entry); long id=num(v,"HeroId",1,Integer.MAX_VALUE); if(!BackupCatalog.has(BackupCatalog.HEROES,id)||!heroes.add(id)) throw invalid(); long level=num(v,"Level",1,100),star=num(v,"Star",1,8); boolean found=false;for(int[] cfg:BackupCatalog.HERO_STARS)if(cfg[0]==id && cfg[1]==star && level<=cfg[2])found=true;if(!found)throw invalid();ints(list(v,"HeroTalent")); }
        if (!heroes.contains(equipped)) throw invalid(); List<Object> guideIds=list(models.get(FILES[6]),"GuideIds"); ints(guideIds); for(Object id:guideIds)if(!BackupCatalog.has(BackupCatalog.GUIDES,(Long)id))throw invalid();
        Map<String,Object> shop=models.get(FILES[7]); num(shop,"LastRefreshTime",0,Long.MAX_VALUE); Set<Long> chests=new HashSet<>();
        for(Object entry:list(shop,"List")) { Map<String,Object> v=obj(entry); if(!chests.add(num(v,"ChestId",1,Integer.MAX_VALUE))) throw invalid(); for(String k:new String[]{"DayRemainNum","NextTime","TotalOpenTimes"})num(v,k,0,Long.MAX_VALUE); List<Object> ids=list(v,"GroupIds"), ns=list(v,"GroupNums"); ints(ids); ints(ns); if(ids.size()!=ns.size())throw invalid(); }
        if (!chests.containsAll(Arrays.asList(101L,108L,115L))) throw invalid();
        if (models.containsKey(FILES[8])) { Map<String,Object> v=models.get(FILES[8]); num(v,"LastClaim",1,Long.MAX_VALUE); num(v,"QuickReturnDay",0,Long.MAX_VALUE); num(v,"QuickReturnCount",0,3); }
        if (models.containsKey(FILES[9])) { Map<String,Object> v=models.get(FILES[9]); num(v,"Date",20000101,99991231); if(!bool(v,"Claimed") || num(v,"Amount",4000,4000)!=4000)throw invalid(); }
        Set<String> keys=new HashSet<>();
        for(Object entry:list(models.get(FILES[10]),"Values")) {
            Map<String,Object> v=obj(entry); Object key=v.get("Key");
            if(!(key instanceof String) || !((String)key).matches("1fight_fail_count|1drop_die_hint|boxguide_close_1|_manga_state|fail_region_[1-9][0-9]{0,9}") || !keys.add((String)key))throw invalid();
            boolean present=bool(v,"Present"); long value=num(v,"Value",0,key.equals("_manga_state")?1:Integer.MAX_VALUE); if(!present && value!=0)throw invalid();
        }
        if (!keys.containsAll(Arrays.asList("1fight_fail_count","1drop_die_hint","boxguide_close_1","_manga_state")) || keys.size()!=4+BackupCatalog.REGIONS.length)throw invalid();
        for(int region:BackupCatalog.REGIONS)if(!keys.contains("fail_region_"+region))throw invalid();
    }

    /** Strict JSON subset used by the bridge: UTF-8, duplicate rejection, bounded depth, integer numbers. */
    private static final class Json {
        final String s; int i;
        Json(byte[] bytes) throws IOException {
            try { s=StandardCharsets.UTF_8.newDecoder().decode(java.nio.ByteBuffer.wrap(bytes)).toString(); }
            catch(Exception e){throw invalid();}
        }
        Object parse() throws IOException { Object v=value(0); ws(); if(i!=s.length())throw invalid(); return v; }
        void ws(){while(i<s.length() && " \r\n\t".indexOf(s.charAt(i))>=0)i++;}
        boolean take(char c){ws();if(i<s.length()&&s.charAt(i)==c){i++;return true;}return false;}
        Object value(int depth)throws IOException {
            if(depth>24)throw invalid(); ws(); if(i==s.length())throw invalid(); char c=s.charAt(i);
            if(c=='{'){i++; Map<String,Object> m=new LinkedHashMap<>(); if(take('}'))return m; do{ws();String k=str();if(k.startsWith("$")||m.containsKey(k)||!take(':'))throw invalid();m.put(k,value(depth+1));}while(take(','));if(!take('}'))throw invalid();return m;}
            if(c=='['){i++;List<Object>a=new ArrayList<>();if(take(']'))return a;do{a.add(value(depth+1));if(a.size()>16384)throw invalid();}while(take(','));if(!take(']'))throw invalid();return a;}
            if(c=='"')return str();
            for(String literal:new String[]{"true","false","null"})if(s.startsWith(literal,i)){i+=literal.length();return literal.equals("null")?null:Boolean.valueOf(literal);}
            int start=i;if(c=='-')i++;if(i>=s.length()||s.charAt(i)<'0'||s.charAt(i)>'9')throw invalid();
            if(s.charAt(i)=='0')i++;else while(i<s.length()&&s.charAt(i)>='0'&&s.charAt(i)<='9')i++;
            try{return Long.valueOf(s.substring(start,i));}catch(Exception e){throw invalid();}
        }
        String str()throws IOException {
            if(i>=s.length()||s.charAt(i++)!='"')throw invalid();StringBuilder b=new StringBuilder();
            while(i<s.length()){char c=s.charAt(i++);if(c=='"'){for(int j=0;j<b.length();j++){char u=b.charAt(j);if(Character.isHighSurrogate(u)){if(++j>=b.length()||!Character.isLowSurrogate(b.charAt(j)))throw invalid();}else if(Character.isLowSurrogate(u))throw invalid();}return b.toString();}if(c<32)throw invalid();if(c=='\\'){if(i==s.length())throw invalid();c=s.charAt(i++);int at="\"\\/bfnrt".indexOf(c);if(at>=0)c="\"\\/\b\f\n\r\t".charAt(at);else if(c=='u'){if(i+4>s.length())throw invalid();for(int j=i;j<i+4;j++)if(Character.digit(s.charAt(j),16)<0)throw invalid();try{c=(char)Integer.parseInt(s.substring(i,i+4),16);}catch(Exception e){throw invalid();}i+=4;}else throw invalid();}b.append(c);if(b.length()>65536)throw invalid();}throw invalid();
        }
    }
}
