# Setup do workspace leve

## Dependências locais explícitas

Copie `config/local.example.json` para `config/local.json` e preencha caminhos. Caminhos relativos são resolvidos na raiz do checkout. A configuração pessoal é ignorada pelo Git; não contém senha em texto, somente o caminho do arquivo privado existente. O helper `tools/workspace/paths.ps1` apenas resolve caminhos, sem ler a chave/senha.

| Chave | Uso |
|---|---|
| `archiveRoot` | Consulta opcional de referências externas no Atlas |
| `originalApk` | Artefato 1.9.1, conferido por SHA256 antes de empacotar |
| `ndkDirectory` | NDK ARM64 utilizado pelo build nativo |
| `toolchainDirectory` | JDK/SDK/D8/zipalign/apksigner/ADB existentes, com `paths.json` |
| `keystore`, `storePasswordFile` | Caminhos privados de assinatura; preservar identidade das instalações existentes |
| `methodMap` | Índice recuperado opcional; Atlas funciona sem ele e declara a ausência |

Nesta máquina, os caminhos apontam explicitamente para o acervo `D:\Programacao\kinja-run`. Nenhum binário ou chave foi copiado. Não exclua o acervo enquanto ele for usado por essa configuração. Em outro computador, provisione as dependências e altere o JSON; não há fallback oculto para a máquina antiga. Não foram adicionadas junctions.

Os scripts usam Python 3 e **Windows PowerShell 5.1** (o PowerShell 7 não está instalado nesta máquina; os scripts funcionam nos dois). Não há compilador C++ de host: os testes C++ são compilados com o clang do NDK e executados só com `-Serial` no emulador (ver [tests/README.md](../../tests/README.md)). Scripts `.ps1` com texto acentuado precisam ser salvos em UTF-8 **com BOM**, senão o 5.1 lê como ANSI. `package.json`/`package-lock.json` preservam a dependência REA histórica; `npm ci` só é necessário para esse tooling, não para compilar, testar ou abrir o Atlas. Scripts de análise em `tools/metadata`, `tools/bundles` e `tools/native` são históricos (ver [tools/README.md](../../tools/README.md)): não executá-los só para preencher pastas vazias ou refazer trabalho já realizado.

## Comandos

```powershell
# Fonte ARM64, sem opções de QA
./offline/native/build-native.ps1

# Contrato do wrapper, diretório de saída deve ser novo
./offline/prepare_wrapper.ps1 -OutputDirectory ./build/wrapper-check-novo

# Atlas; não executa engenharia reversa
python project-atlas/build_inventory.py
```

Para gerar a build normal, siga [RELEASE.md](RELEASE.md). Empacotamento manual usa `offline/build.ps1 -BuildId <novo> -PackageName <package-alvo> -NativeLibrary ./offline/build/arm64-v8a/libkinjaoffline.so -PackageApk`. O package default histórico do script não identifica automaticamente seu alvo: informe explicitamente. Saídas são criadas no checkout novo; original e toolchain são lidos dos caminhos configurados. O script de assinatura não instala o APK.

Hash esperado do APK original: `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`. O guard original permanece no empacotamento.

## Git e painel

`git status --short --untracked-files=all` mostra material candidato a versionamento. `.gitignore` exclui APKs, chave/senha, toolchains, bibliotecas, saídas, índice gerado e configuração pessoal. Não executar `git add -f` para contornar essas regras. Antes de publicação, revisar o diff e a titularidade do que será publicado.

O Atlas indexa o checkout e identifica referências centrais que só existem no acervo externo. Métodos do índice recuperado podem ser consultados sem copiar o índice-fonte para cá. `inventory.js`/`inventory.json` são gerados locais e podem conter caminhos absolutos; não são publicados automaticamente. Sem configuração/índice externo, os sistemas, arquivos de fonte, documentos e hooks continuam navegáveis; referências ausentes são declaradas.

O empacotamento também recusa BuildIds já presentes em `archiveRoot/build`, além das saídas locais. A pasta nova não reinicia o histórico de versões.

## Limpeza após concluir testes

Manter somente o APK normal atual e seu relatório de assinatura em `build`, conforme preferência do usuário. Após documentar a validação e encerrar os testes da versão:

```powershell
./tools/workspace/clean-builds.ps1 -KeepBuildId <build-normal-atual> -WhatIf
./tools/workspace/clean-builds.ps1 -KeepBuildId <build-normal-atual>
```

Nas próximas versões, informar o novo BuildId normal. A limpeza remove os outros APKs, unsigned, intermediários e evidências brutas locais; não usar no meio de uma investigação que ainda precisa desses arquivos. Mantém as identidades pequenas em `docs/development/build-history.json`, consultado pelo empacotamento para não reutilizar IDs apagados. Não remove acervo externo, dependências, credenciais ou saves Android. O script recusa links/arquivos desconhecidos e confere o hash do APK mantido. Relatórios versionados continuam como síntese histórica, mesmo quando seus logs/capturas antigos já foram removidos.
