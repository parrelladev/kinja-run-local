package local.kinja;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.app.AlertDialog;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.system.Os;
import android.system.OsConstants;
import android.view.Gravity;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;
import java.io.*;
import java.util.Arrays;
import com.unity3d.player.UnityPlayerActivity;

/** Launcher subclass retaining all original Unity lifecycle behavior. */
public final class OfflineActivity extends UnityPlayerActivity {
    private static AssetManager assetManager;
    private static OfflineActivity current;
    private static boolean processBootChecked;
    private BackupStore backups;
    private Button backupEntry;
    private AlertDialog backupDialog;
    private byte[] snapshot, exporting;
    private String snapshotError;
    private boolean importQueued, busy;
    private static final int EXPORT = 4811, IMPORT = 4812;

    static {
        System.loadLibrary("kinjaoffline");
    }

    private static native void configure(String filesDirectory, AssetManager manager);

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

    /** JNI entry, called synchronously on Unity thread after UISetting.DoOpen. */
    public static void settingsOpened(String preferences) {
        OfflineActivity a=current; if(a==null)return;
        byte[] captured=null; String error=null;
        try { captured=BackupStore.encode(BackupStore.snapshot(new File(a.getFilesDir(),"offline_save"),preferences)); }
        catch(IOException e){error=e.getMessage();}
        final byte[] bytes=captured; final String failure=error;
        a.runOnUiThread(()->{
            a.snapshot=bytes; a.snapshotError=failure;
            if(a.backupEntry!=null)return;
            Button button=new Button(a); button.setText("Backup do progresso"); button.setTextColor(Color.WHITE);
            button.setTypeface(Typeface.DEFAULT,Typeface.BOLD); button.setAllCaps(false); button.setTextSize(16);
            button.setBackground(a.panel(true)); button.setPadding(a.dp(20),a.dp(8),a.dp(20),a.dp(8));
            button.setOnClickListener(v->a.openBackup());
            FrameLayout.LayoutParams p=new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,a.dp(52),Gravity.BOTTOM|Gravity.CENTER_HORIZONTAL);
            p.bottomMargin=a.dp(20); a.addContentView(button,p); a.backupEntry=button;
        });
    }
    public static void settingsClosed() {
        OfflineActivity a=current;if(a==null)return;
        a.runOnUiThread(()->{
            a.snapshot=null;
            if(a.backupEntry!=null){((ViewGroup)a.backupEntry.getParent()).removeView(a.backupEntry);a.backupEntry=null;}
            if(a.backupDialog!=null)a.backupDialog.dismiss();
        });
    }
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
    private int dp(int value){return Math.round(value*getResources().getDisplayMetrics().density);}
    private GradientDrawable panel(boolean blue){
        GradientDrawable g=new GradientDrawable(GradientDrawable.Orientation.TOP_BOTTOM,blue?new int[]{0xff8baeff,0xff5279ed}:new int[]{0xff414255,0xff303140});
        g.setCornerRadius(dp(12));g.setStroke(dp(3),blue?0xff283d88:0xff20212d);return g;
    }
    private void message(String title,String text){runOnUiThread(()->new AlertDialog.Builder(this).setTitle(title).setMessage(text).setPositiveButton("OK",null).show());}
    private void openBackup(){
        if(busy || importQueued)return;
        LinearLayout body=new LinearLayout(this);body.setOrientation(LinearLayout.VERTICAL);body.setPadding(dp(20),dp(18),dp(20),dp(20));body.setBackground(panel(false));
        TextView title=new TextView(this);title.setText("Backup do progresso");title.setTextColor(Color.WHITE);title.setTextSize(21);title.setTypeface(Typeface.DEFAULT,Typeface.BOLD);body.addView(title);
        TextView detail=new TextView(this);detail.setText("Guarde uma cópia em Arquivos e transfira para outro celular. A importação substitui o progresso atual e exige reabrir o jogo.");detail.setTextColor(0xffe4e6f3);detail.setPadding(0,dp(12),0,dp(12));body.addView(detail);
        for(int kind=0;kind<2;kind++){
            final boolean export=kind==0;Button b=new Button(this);b.setText(export?"Exportar backup":"Importar backup");b.setTextColor(Color.WHITE);b.setTypeface(Typeface.DEFAULT,Typeface.BOLD);b.setAllCaps(false);b.setBackground(panel(true));
            LinearLayout.LayoutParams p=new LinearLayout.LayoutParams(-1,dp(52));p.topMargin=dp(10);body.addView(b,p);
            b.setOnClickListener(v->{backupDialog.dismiss();if(export)startExport();else startImport();});
        }
        backupDialog=new AlertDialog.Builder(this).setView(body).setNegativeButton("Voltar",null).create();backupDialog.show();
    }
    private void startExport(){
        if(snapshot==null){message("Não foi possível exportar",snapshotError==null?"Reabra as Configurações para capturar o progresso.":snapshotError);return;}
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
