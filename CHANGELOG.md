# Histórico de alterações

Todas as alterações relevantes do **samp-discord-connector-modern** serão registradas neste arquivo.

## 0.4.0 — 18/09/2026

Primeira versão moderna desenvolvida inicialmente para o **Rocha Roleplay** e disponibilizada publicamente à comunidade SA-MP/open.mp.

### Adicionado

- suporte a botões nativos do Discord;
- suporte a Action Rows;
- String Select Menu;
- User Select Menu;
- Role Select Menu;
- Mentionable Select Menu;
- Channel Select Menu;
- modais nativos com a estrutura atual de Label;
- campos Text Input curtos e em parágrafo;
- selects dentro de modais;
- Text Display dentro de modais;
- File Upload;
- Radio Group;
- Checkbox Group;
- Checkbox individual;
- callback `DCC_OnButtonInteraction`;
- callback `DCC_OnSelectInteraction`;
- callback `DCC_OnModalSubmit`;
- leitura de `custom_id`;
- leitura de valores selecionados;
- leitura de campos enviados por modal;
- respostas efêmeras em interações;
- atualização da mensagem de origem da interação;
- envio de mensagens comuns e embeds contendo componentes;
- workflow de compilação para Windows x86 e Linux i386.

### Mantido

- API `DCC_*` existente sempre que possível;
- suporte a SA-MP;
- suporte a open.mp;
- comandos/interactions da base v0.3.6;
- embeds, canais, mensagens, usuários, cargos, guilds, emojis e reações.

### Base

Projeto derivado do `maddinat0r/samp-discord-connector`, preservando a licença MIT e os créditos originais.
