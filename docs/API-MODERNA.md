# API moderna de componentes

Este documento resume a camada adicionada pelo **samp-discord-connector-modern**. As assinaturas definitivas continuam sendo as declaradas em `discord-connector.inc`.

## Componentes

Uma mensagem do Discord pode conter até cinco Action Rows. Cada linha pode receber botões ou um menu de seleção compatível.

### Botões

Crie um botão com `DCC_CreateButton`, adicione-o a uma linha com `DCC_AddComponentToRow` e envie a linha usando `DCC_SendChannelMessageComponents` ou `DCC_SendChannelEmbedMessageComponents`.

Estilos disponíveis:

- `DCC_BUTTON_PRIMARY`
- `DCC_BUTTON_SECONDARY`
- `DCC_BUTTON_SUCCESS`
- `DCC_BUTTON_DANGER`
- `DCC_BUTTON_LINK`

Cliques são entregues em:

```pawn
forward DCC_OnButtonInteraction(
    DCC_Interaction:interaction,
    DCC_User:user,
    const custom_id[]
);
```

## Menus de seleção

Tipos suportados:

- String;
- User;
- Role;
- Mentionable;
- Channel.

Use `DCC_GetInteractionValueCount` e `DCC_GetInteractionValue` para recuperar os valores enviados pelo Discord.

## Modais

Crie o modal usando `DCC_CreateModal`, adicione campos com `DCC_AddModalTextInput` e abra-o em resposta a uma interação usando `DCC_ShowModal`.

O envio é recebido em `DCC_OnModalSubmit`. Use `DCC_GetInteractionModalValue` informando o `custom_id` do campo para recuperar o conteúdo.

## Respostas

`DCC_SendInteractionMessage` aceita o argumento `ephemeral`. Quando `true`, a resposta inicial fica visível apenas para o usuário que executou a interação.

`DCC_UpdateInteractionMessage` permite substituir o conteúdo textual da mensagem ligada à interação.

## Memória dos componentes

Componentes e modais são objetos internos do plugin. Quando não forem mais necessários, libere-os com:

```pawn
DCC_DeleteComponent(componente);
DCC_DeleteModal(modal);
```

Excluir o objeto interno depois que a requisição já foi montada/enviada não remove o componente da mensagem publicada no Discord.
