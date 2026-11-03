# 0001 — Checkout leve e acervo externo

Decisão autorizada pelo usuário nesta conversa: migrar somente material de trabalho para `kinja-run-local`. Não duplicar os cerca de 90 GB de builds.

Manter paths relativos das fontes, copiar seletivamente com SHA256 e preservar a origem. Fonte de gameplay, wrapper Java, planejador e seeds permanecem byte a byte iguais. Alterações permitidas nesta migração são resolução de caminhos dos scripts, portabilidade do Atlas, ignore e documentação.

Dependências binárias, original APK, assinatura e extrações já recuperadas continuam externas e são configuradas explicitamente por JSON local ignorado. Não usar symlinks, não gerar nova assinatura, não limpar save, não executar engenharia reversa e não instalar APK como efeito colateral.

Consequência: checkout pode ser leve e legível no GitHub; uma build requer provisionamento externo documentado. BlueStacks é ambiente planejado, sem validação transferida do telefone. `.git` e `.gitattributes` existentes são preservados; nenhum push é parte da migração.
