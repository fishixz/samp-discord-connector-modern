#include <a_samp>
#include <discord-connector>

new DCC_Channel:gCanal;

public DCC_OnMessageCreate(DCC_Message:message)
{
    return 1;
}

stock EnviarExemploComponentes(DCC_Channel:canal)
{
    new DCC_Component:linha = DCC_CreateActionRow();
    new DCC_Component:botao = DCC_CreateButton(
        "rocha_cadastro",
        "Abrir cadastro",
        DCC_BUTTON_PRIMARY
    );

    DCC_AddComponentToRow(linha, botao);
    DCC_SendChannelMessageComponents(
        canal,
        "Clique no botão abaixo para testar:",
        linha
    );

    DCC_DeleteComponent(botao);
    DCC_DeleteComponent(linha);
    return 1;
}

public DCC_OnButtonInteraction(
    DCC_Interaction:interaction,
    DCC_User:user,
    const custom_id[]
)
{
    if (!strcmp(custom_id, "rocha_cadastro"))
    {
        new DCC_Modal:modal = DCC_CreateModal(
            "rocha_cadastro_modal",
            "Cadastro"
        );

        DCC_AddModalTextInput(
            modal,
            "nome",
            "Seu nome",
            DCC_TEXT_INPUT_SHORT,
            "Digite seu nome"
        );

        DCC_ShowModal(interaction, modal);
        DCC_DeleteModal(modal);
    }
    return 1;
}

public DCC_OnModalSubmit(
    DCC_Interaction:interaction,
    DCC_User:user,
    const custom_id[]
)
{
    if (!strcmp(custom_id, "rocha_cadastro_modal"))
    {
        new nome[64];

        if (DCC_GetInteractionModalValue(
            interaction,
            "nome",
            nome,
            sizeof nome
        ))
        {
            DCC_SendInteractionMessage(
                interaction,
                "Cadastro recebido com sucesso.",
                true
            );
        }
    }
    return 1;
}
