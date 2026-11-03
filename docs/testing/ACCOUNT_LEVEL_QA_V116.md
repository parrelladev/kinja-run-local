# Nível da conta — teste com saves sintéticos, 03/11/2026

O usuário autorizou criar um save editado para testar o bug sem precisar progredir do zero. Foram criados **dois packages QA independentes** no BlueStacks; não houve importação/leitura do save do celular, alteração do acervo externo, limpeza de dados ou seeding no audit1. O capítulo/nível desses perfis são artificiais, não evidência de progressão natural.

## Reprodução e resultado observado

| Caso | Estado inicial artificial | Ação no jogo original | Resultado |
|---|---|---|---|
| Migração do excesso | Nível 2, EXP 5838, ouro 1500, gemas 0, inventário vazio | Login v113, depois force-stop/start | Nível 4, EXP 838; PID7639→8592 preservou os valores, sem uma segunda promoção |
| Nova concessão | Nível 2, EXP 1990, ouro 1500, capítulos 1/2 concluídos para expor HangUp | Uma coleta Receber na v115 | 151 ouro/151 EXP; nível 3, EXP 141/3000, ouro 1651, gemas 0 |
| Persistência da concessão | Resultado acima | Force-stop/start PID9290→10242 | Home/logs novamente nível 3, EXP 141 e ouro 1651; sem repetição da recompensa |

Na coleta, o painel inicialmente mostrou 150/150; pelo tempo decorrido até o clique, a concessão calculada foi 151/151. Log: `Local account level advanced level=2->3 exp=1990+151->141`; readback original confirmou ouro1651/gemas0/EXP141 e o nível foi conferido pelo getter. A home mostrou nível3 e 141/3000 na mesma sessão, antes do reinício. Menus foram recalculados com nível3; não demonstra um novo desbloqueio porque o fixture já satisfazia os quatro menus locais.

## Defeito adicional encontrado na v112

Na v113 (biblioteca idêntica à v112), interações concorrentes no perfil de migração chegaram ao resgate diário da Loja e expuseram `Cannot read live account level`. Elas não foram parte da coleta controlada, nem foram usadas como teste positivo; o ouro permaneceu 1500 no reinício. A leitura adicionada na v112 chamava `ObscuredInt.GetDecrypted` com um receiver boxed. `runtime_invoke` precisa do valor unboxed para um método de instância desse struct.

Correção mínima: `api.object_unbox` antes de invocar `GetDecrypted`, sem escrever representação interna ou offsets. Biblioteca corrigida SHA256 `370F21519AA454816D373F0BE1BFC24388DD3AD2CED48A0C9125BA3FF93E7DCF`. A v115 validou o caminho completo de leitura, crédito, setter de nível, sync, readback, gravação e restart. **Usar v116 normal em lugar da v112**, que pode recusar créditos por essa leitura incorreta.

## Fixtures reproduzíveis

`offline/build.ps1 -AccountProgressionFixture Migration|RewardBoundary` cria cópias dos seeds somente no diretório novo de build. O default `None` preserva os seeds normais. Cada fixture exige seu package exato; tentativa de usar `audit1` foi recusada antes de criar outputs. Não há código de reposição no runtime: o seed vale apenas na primeira abertura de uma instalação nova, mantendo o save nos updates. `PlayerName` e label do APK identificam QA. Não usar esses APKs como perfil natural.

```powershell
# Em nova reprodução, escolher BuildIds inéditos; os abaixo já existem.
./offline/build.ps1 -BuildId NOVO-migration -PackageName com.habby.kinjarun.offline.qa.expmigration -AccountProgressionFixture Migration -NativeLibrary ./offline/build/arm64-v8a/libkinjaoffline.so -PackageApk
./offline/build.ps1 -BuildId NOVO-reward -PackageName com.habby.kinjarun.offline.qa.expreward -AccountProgressionFixture RewardBoundary -NativeLibrary ./offline/build/arm64-v8a/libkinjaoffline.so -PackageApk
```

## Builds e evidências

| Build | Package | APK SHA256 |
|---|---|---|
| v113-qa-exp-migration | qa.expmigration | CEA28002A999DC3F7BD0C5EFFA6B5FFC793F34F995A2A4F773B385A21AB5F7DB |
| v114-qa-exp-reward-boundary | qa.expreward | 051C980EF4C4F503F681A484A8AF65F1BE48181F969A60C62D4E61B6CADC507D |
| v115-qa-exp-reward-unboxed | qa.expreward | 0BBE051700FCFF6E5864E26E457009099F364CA195F1A465A7CFEA728C2C3B0F |
| v116-account-exp-unboxed-level | audit1, seeds normais | 3F51518B756096B19B32E25C7FA5B1EAC10AE31316A4E11AF6761E5445500BC6 |
| v117-qa-exp-migration-unboxed | qa.expmigration | 28ABECD2DDEC56B6A9CCC76B1237D8A35304D6913668A47FC9640D68EB687099 |

Todos usam prefixo `com.habby.kinjarun.offline.`. V113/v114 usavam a biblioteca v112; v114 não chegou a ser aberta antes da atualização. V115/v116/v117 têm a biblioteca corrigida idêntica; v117 atualizou o fixture já migrado, sem recriá-lo. Assinatura de desenvolvimento existente mantida. V116 atualizada no audit1 por `install -r`; telefone não atualizado.

Audit1/v116 foi aberto depois do QA: initialized, nível1, ouro1500/gemas0/EXP0 e menus bloqueados, sem herdar dados artificiais. `firstInstallTime=2026-11-03 13:45:26` preservado. Ao concluir, perfil QA de recompensa reaberto para inspeção do usuário.

Logs e capturas estão em `build/tests/account-exp-qa/`, ignorado pelo Git: `migration-start.txt`, `migration-restart.txt/png`, `reward-before.txt/png`, `hangup-panel.png`, `reward-claim.txt/png`, `reward-after-home.png`, `reward-restart.txt/png`. Login opcional do Play Games foi cancelado no fixture de recompensa, sem autenticação/sincronização. Rede do emulador permaneceu inalterada.

Limites: teste positivo de EXP já acumulada e nova concessão via HangUp, não de fórmula de EXP por combate, recompensa de level-up, progressão natural longa, save real do celular, recuperação sob falha injetada ou dos outros bugs. Permanecer no bug de nível até avaliação do usuário.
