package local.kinja;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.app.AlertDialog;
import android.content.Intent;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.system.Os;
import android.system.OsConstants;
import java.io.*;
import com.unity3d.player.UnityPlayerActivity;

/** Launcher subclass retaining all original Unity lifecycle behavior. */
public final class OfflineActivity extends UnityPlayerActivity {
    private static AssetManager assetManager;
    private static OfflineActivity current;
    private static boolean processBootChecked;
    private static volatile boolean nativeBackupVisible;
    private BackupStore backups;
    private byte[] snapshot, exporting;
    private String snapshotError;
    private volatile boolean importQueued, busy;
    private static final int EXPORT = 4811, IMPORT = 4812;

    static {
        System.loadLibrary("kinjaoffline");
    }

    private static native void configure(String filesDirectory, AssetManager manager);
    private static native void closeBackupWindow() throws IOException;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        current = this;
        String notice = null;
        try {
        backups = new BackupStore(getFilesDir(), new BackupStore.Disk() {
            public void syncDirectory(File dir) throws IOException {
                java.io.FileDescriptor fd = null;
                try { fd = Os.open(dir.getAbsolutePath(), OsConstants.O_RDONLY, 0); Os.fsync(fd); }
                catch (Exception e) { throw new IOException("Não foi possível confirmar a gravação local.", e); }
                finally { if (fd != null) try { Os.close(fd); } catch (Exception ignored) {} }
            }
            public void point(String name) {} // fault injection exists only in host fixtures
            public void rename(File from,File to) throws IOException {
                try { Os.rename(from.getAbsolutePath(),to.getAbsolutePath()); }
                catch(Exception e){throw new IOException("Não foi possível substituir o progresso com segurança.",e);}
            }
        });
        if(!processBootChecked){notice = backups.boot();processBootChecked=true;}
        }
        catch (IOException e) {
            // Fail closed: do not start the bridge or Unity on an uncertain save.
            android.widget.Toast.makeText(this,"Backup: "+e.getMessage()+" Reabra o jogo para recuperar o progresso anterior.",android.widget.Toast.LENGTH_LONG).show();
            android.os.Process.killProcess(android.os.Process.myPid());
            throw new IllegalStateException("Backup recovery blocked Unity startup",e);
        }
        assetManager = getAssets();
        configure(getFilesDir().getAbsolutePath(), assetManager);
        super.onCreate(savedInstanceState);
        if (notice != null) message("Backup do progresso", notice);
    }

    /** Called by native NGUI actions on UnityMain; the snapshot cannot interleave game writes. */
    public static synchronized void backupAction(String payload) {
        OfflineActivity a=current;if(a==null || payload==null || payload.length()<2)return;
        int action=payload.charAt(0)-'0';
        if(action==0 || action==1){nativeBackupVisible=action==1;return;}
        if(action==4){a.message("Backup indisponível",payload.substring(2));return;}
        if(a.busy || a.importQueued)return;
        a.busy=true;
        if(action==2){
            try{
                a.snapshot=BackupStore.encode(BackupStore.snapshot(new File(a.getFilesDir(),"offline_save"),payload.substring(2)));
                a.snapshotError=null;a.runOnUiThread(()->a.startExport());
            }catch(IOException e){a.busy=false;a.message("Não foi possível exportar",e.getMessage());}
        }else if(action==3)a.runOnUiThread(()->a.startImport());
        else a.busy=false;
    }
    private void closeNativeBackup(){
        try{
            java.lang.reflect.Field field=UnityPlayerActivity.class.getDeclaredField("mUnityPlayer");field.setAccessible(true);
            Object player=field.get(this);
            java.lang.reflect.Method queue=player.getClass().getDeclaredMethod("queueGLThreadEvent",Runnable.class);queue.setAccessible(true);
            queue.invoke(player,(Runnable)()->{try{closeBackupWindow();}catch(IOException e){message("Backup",e.getMessage());}});
        }catch(Exception e){message("Backup","Use o X ou toque fora da janela para fechar.");}
    }
    @Override public boolean onKeyDown(int key,android.view.KeyEvent event){
        if(key==android.view.KeyEvent.KEYCODE_BACK && nativeBackupVisible)return true;
        return super.onKeyDown(key,event);
    }
    @Override public boolean onKeyUp(int key,android.view.KeyEvent event){
        if(key==android.view.KeyEvent.KEYCODE_BACK && nativeBackupVisible){closeNativeBackup();return true;}
        return super.onKeyUp(key,event);
    }
    @Override public void onBackPressed(){if(nativeBackupVisible)closeNativeBackup();else super.onBackPressed();}
    /** Original preference APIs are invoked by C++ on Unity thread, never by the UI worker. */
    public static String preparePreferences(String currentPreferences) throws IOException {
        if(current==null)throw new IOException("Activity indisponível.");
        return current.backups.preferences(currentPreferences);
    }
    public static void bootstrapComplete() throws IOException {
        OfflineActivity a=current;if(a==null)throw new IOException("Activity indisponível.");
        if(a.backups.initialized())a.runOnUiThread(()->a.message("Backup importado","O progresso foi substituído e carregado corretamente."));
    }
    public static void bootstrapFailed() {
        OfflineActivity a=current;if(a==null)return;
        try {
            if(a.backups.applying())a.runOnUiThread(()->new AlertDialog.Builder(a).setTitle("Importação não concluída")
                .setMessage("O progresso do backup não pôde ser carregado. Feche e reabra o jogo para restaurar o progresso anterior.")
                .setCancelable(false).setPositiveButton("Fechar e recuperar",(d,w)->a.closeProcess()).show());
        } catch(IOException e){a.message("Falha na importação","Feche e reabra o jogo para verificar a recuperação.");}
    }
    private void message(String title,String text){runOnUiThread(()->new AlertDialog.Builder(this).setTitle(title).setMessage(text).setPositiveButton("OK",null).show());}
    private void startExport(){
        if(snapshot==null){message("Não foi possível exportar",snapshotError==null?"Reabra a janela de Backup para capturar o progresso.":snapshotError);return;}
        exporting=snapshot.clone();busy=true;
        try{
            Intent i=new Intent(Intent.ACTION_CREATE_DOCUMENT).setType("application/octet-stream").addCategory(Intent.CATEGORY_OPENABLE)
                .putExtra(Intent.EXTRA_LOCAL_ONLY,true).putExtra(Intent.EXTRA_TITLE,"Kinja-Run-"+new java.text.SimpleDateFormat("yyyyMMdd-HHmmss",java.util.Locale.ROOT).format(new java.util.Date())+".kinjabak");
            startActivityForResult(i,EXPORT);
        }catch(Exception e){busy=false;exporting=null;message("Não foi possível exportar","Nenhum seletor de arquivos está disponível.");}
    }
    private void startImport(){
        busy=true;
        try{startActivityForResult(new Intent(Intent.ACTION_OPEN_DOCUMENT).setType("*/*").addCategory(Intent.CATEGORY_OPENABLE).putExtra(Intent.EXTRA_LOCAL_ONLY,true),IMPORT);}
        catch(Exception e){busy=false;message("Não foi possível importar","Nenhum seletor de arquivos está disponível.");}
    }
    @Override protected void onActivityResult(int request,int result,Intent data){
        super.onActivityResult(request,result,data);
        if(request!=EXPORT && request!=IMPORT)return;
        if(result!=RESULT_OK || data==null || data.getData()==null){busy=false;exporting=null;return;}
        final Uri uri=data.getData();final byte[] frozen=exporting;exporting=null;
        new Thread(()->{
            try{
                if(request==EXPORT){
                    if(frozen==null)throw new IOException("A captura foi perdida. Reabra as Configurações.");
                    BackupStore.export(frozen,new BackupStore.ExportFile(){
                        public void writeAndSync(byte[] bytes)throws IOException{
                            ParcelFileDescriptor fd=getContentResolver().openFileDescriptor(uri,"rwt");
                            if(fd==null)throw new IOException("Não foi possível abrir o destino.");
                            try(ParcelFileDescriptor.AutoCloseOutputStream out=new ParcelFileDescriptor.AutoCloseOutputStream(fd)){out.write(bytes);out.flush();out.getFD().sync();}
                        }
                        public byte[] readBack()throws IOException{
                            try(InputStream in=getContentResolver().openInputStream(uri)){if(in==null)throw new IOException("Não foi possível conferir o destino.");return BackupStore.read(in,BackupStore.LIMIT*12);}
                        }
                    });
                    message("Backup exportado","O arquivo foi gravado e conferido. Você pode guardá-lo ou transferi-lo para outro celular.");
                }else{
                    byte[] bytes;
                    try(InputStream in=getContentResolver().openInputStream(uri)){if(in==null)throw BackupStore.invalid();bytes=BackupStore.read(in,BackupStore.LIMIT*12);}
                    BackupStore.decode(bytes);
                    final byte[] validated=bytes;
                    runOnUiThread(()->new AlertDialog.Builder(this).setTitle("Substituir todo o progresso?")
                        .setMessage("Conta, capítulos, heróis, itens e moedas atuais serão substituídos pelo backup selecionado. O jogo será fechado e a importação será aplicada ao reabrir. Uma cópia privada de recuperação protegerá o estado anterior.")
                        .setNegativeButton("Cancelar",null).setPositiveButton("Substituir e fechar",(d,w)->queueImport(validated)).show());
                }
            }catch(Exception e){
                if(request==EXPORT)try{DocumentsContract.deleteDocument(getContentResolver(),uri);}catch(Exception ignored){}
                message(request==EXPORT?"Backup não exportado":"Backup não importado",e instanceof IOException?e.getMessage():"Falha ao acessar o arquivo. Verifique o espaço e a permissão. O progresso atual foi preservado.");
            }finally{runOnUiThread(()->busy=false);}
        },"KinjaManualBackup").start();
    }
    private void queueImport(byte[] backup){
        if(importQueued)return;busy=true;
        new Thread(()->{
            try{backups.request(backup);importQueued=true;runOnUiThread(()->closeProcess());}
            catch(IOException e){message("Importação não agendada",e.getMessage()+" O progresso atual foi preservado.");runOnUiThread(()->busy=false);}
        },"KinjaImportRequest").start();
    }
    private void closeProcess(){finishAndRemoveTask();android.os.Process.killProcess(android.os.Process.myPid());}
}
