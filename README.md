# samp-discord-connector-modern

Plugin moderno de integração entre **Discord e SA-MP/open.mp**, permitindo controlar um bot do Discord diretamente pelo **Pawn**.

Este projeto nasceu para atender às necessidades do **Rocha Roleplay**, principalmente recursos que o `samp-discord-connector` clássico não expunha para o Pawn, como **botões, menus de seleção e modais nativos do Discord**. O projeto é público e está liberado para a comunidade utilizar, modificar e contribuir.

> Este é um fork/continuação comunitária baseado no projeto original `maddinat0r/samp-discord-connector`. Os créditos e a licença MIT do projeto original foram preservados.

## Principais novidades

Além dos recursos já existentes no connector original, esta versão adiciona uma camada moderna de interações do Discord:

- Botões nativos do Discord.
- Action Rows para organizar componentes.
- String Select Menu.
- User Select Menu.
- Role Select Menu.
- Mentionable Select Menu.
- Channel Select Menu.
- Modais nativos seguindo a estrutura atual do Discord com Label.
- Text Input curto e em parágrafo.
- Select Menus dentro de modais.
- Text Display dentro de modais.
- File Upload dentro de modais.
- Radio Group.
- Checkbox Group.
- Checkbox individual.
- Respostas efêmeras em interações.
- Atualização da mensagem original após uma interação.
- Leitura do `custom_id` de botões, selects e modais.
- Leitura dos valores selecionados em menus.
- Leitura dos campos enviados por um modal.
- Callbacks Pawn específicos para botão, select e modal.
- Compatibilidade mantida com SA-MP e open.mp.
- Builds automáticos para Linux i386 e Windows x86.

### Callbacks novos

```pawn
forward DCC_OnButtonInteraction(DCC_Interaction:interaction, DCC_User:user, const custom_id[]);
forward DCC_OnSelectInteraction(DCC_Interaction:interaction, DCC_User:user, const custom_id[]);
forward DCC_OnModalSubmit(DCC_Interaction:interaction, DCC_User:user, const custom_id[]);
```

## Recursos herdados

A base original continua disponível, incluindo integração com:

- canais;
- mensagens;
- usuários;
- cargos;
- servidores/guilds;
- embeds;
- emojis e reações;
- comandos/interactions;
- presença do bot;
- SA-MP e open.mp.

A intenção é manter o máximo possível de compatibilidade com scripts que já utilizam as natives `DCC_*` tradicionais.

## Exemplo rápido

Criando uma mensagem com um botão:

```pawn
new DCC_Component:linha = DCC_CreateActionRow();
new DCC_Component:botao = DCC_CreateButton(
    "rocha_teste",
    "Clique aqui",
    DCC_BUTTON_PRIMARY
);

DCC_AddComponentToRow(linha, botao);
DCC_SendChannelMessageComponents(canal, "Teste do Rocha Roleplay:", linha);

DCC_DeleteComponent(botao);
DCC_DeleteComponent(linha);
```

Recebendo o clique e abrindo um modal:

```pawn
public DCC_OnButtonInteraction(DCC_Interaction:interaction, DCC_User:user, const custom_id[])
{
    if (!strcmp(custom_id, "rocha_teste"))
    {
        new DCC_Modal:modal = DCC_CreateModal("rocha_modal", "Cadastro");
        DCC_AddModalTextInput(modal, "nome", "Seu nome");
        DCC_ShowModal(interaction, modal);
        DCC_DeleteModal(modal);
    }
    return 1;
}

public DCC_OnModalSubmit(DCC_Interaction:interaction, DCC_User:user, const custom_id[])
{
    if (!strcmp(custom_id, "rocha_modal"))
    {
        new nome[64];
        DCC_GetInteractionModalValue(interaction, "nome", nome);
        DCC_SendInteractionMessage(interaction, "Dados recebidos!", true);
    }
    return 1;
}
```

Um exemplo pronto também fica disponível em `examples/exemplo_componentes.pwn`.

## Instalação no SA-MP

1. Baixe uma build compatível.
2. Coloque `discord-connector.dll` ou `discord-connector.so` na pasta `plugins`.
3. Coloque `discord-connector.inc` na pasta de includes do Pawn.
4. Carregue o plugin no `server.cfg`:

Windows:

```text
plugins discord-connector
```

Linux:

```text
plugins discord-connector.so
```

5. Configure o token:

```text
discord_bot_token SEU_TOKEN_DO_BOT
```

Também é possível usar a variável de ambiente `DCC_BOT_TOKEN`.

**Nunca publique ou compartilhe o token do seu bot.**

## Instalação no open.mp

Extraia os arquivos da build. Para carregar como componente nativo do open.mp, coloque o conteúdo correspondente em `components`.

No `config.json`:

```json
{
    "discord": {
        "bot_token": "SEU_TOKEN_DO_BOT"
    }
}
```

Também é possível utilizar `DCC_BOT_TOKEN`.

## API moderna

Algumas das principais natives adicionadas:

```pawn
DCC_CreateActionRow
DCC_CreateButton
DCC_CreateStringSelect
DCC_CreateUserSelect
DCC_CreateRoleSelect
DCC_CreateMentionableSelect
DCC_CreateChannelSelect
DCC_AddSelectOption
DCC_AddComponentToRow
DCC_SendChannelMessageComponents
DCC_SendChannelEmbedMessageComponents

DCC_CreateModal
DCC_AddModalTextInput
DCC_AddModalSelect
DCC_AddModalTextDisplay
DCC_CreateFileUpload
DCC_AddFileUploadType
DCC_CreateRadioGroup
DCC_CreateCheckboxGroup
DCC_CreateCheckbox
DCC_AddModalComponent
DCC_ShowModal

DCC_GetInteractionCustomId
DCC_GetInteractionValueCount
DCC_GetInteractionValue
DCC_GetInteractionModalValue
DCC_SendInteractionMessage
DCC_UpdateInteractionMessage
```

A definição completa e atualizada da API está em `discord-connector.inc.in`.

## Compilação

O plugin precisa ser compilado para **32 bits**, seguindo a ABI utilizada pelo SA-MP/open.mp nesse tipo de plugin.

Clone com os submódulos:

```bash
git clone --recursive https://github.com/fishixz/samp-discord-connector-modern.git
cd samp-discord-connector-modern
```

O GitHub Actions deste repositório também compila automaticamente builds x86 para Windows e i386 para Linux.

## Status do projeto

O código é **público e aberto à comunidade**. A implementação moderna está sendo validada por compilação automática antes da primeira release estável. Para servidores em produção, dê preferência às versões publicadas em **Releases** em vez de commits intermediários da branch `main`.

## Origem e créditos

Este projeto utiliza como base o **samp-discord-connector**, originalmente desenvolvido por **Alex Martin (maddinat0r)** e seus contribuidores.

A edição **samp-discord-connector-modern** foi criada inicialmente para o **Rocha Roleplay** e posteriormente disponibilizada publicamente para que outros projetos SA-MP/open.mp também possam utilizar os novos recursos.

Nenhuma autoria anterior é removida ou substituída.

## Licença

Distribuído sob a **Licença MIT**, seguindo a licença da base original.

Consulte o arquivo `LICENSE` para os termos completos.
