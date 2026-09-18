# samp-discord-connector-modern

Modern continuation of `samp-discord-connector` for SA-MP and open.mp, focused on current Discord interactions.

## Goals

- Preserve compatibility with existing `DCC_*` Pawn APIs.
- Discord message buttons.
- String/user/role/channel/mentionable select menus.
- Modal dialogs and text inputs.
- Pawn callbacks for component interactions and modal submissions.
- Linux `discord-connector.so` and Windows builds.
- SA-MP and open.mp compatibility.

## Planned Pawn API

```pawn
DCC_Component:DCC_CreateButton(const custom_id[], const label[], DCC_ButtonStyle:style = DCC_BUTTON_PRIMARY, bool:disabled = false, const emoji[] = "", const url[] = "");
DCC_Component:DCC_CreateStringSelect(const custom_id[], const placeholder[] = "", min_values = 1, max_values = 1, bool:disabled = false);
DCC_AddSelectOption(DCC_Component:select, const label[], const value[], const description[] = "", const emoji[] = "", bool:is_default = false);
DCC_Modal:DCC_CreateModal(const custom_id[], const title[]);
DCC_AddModalTextInput(DCC_Modal:modal, const custom_id[], const label[], DCC_TextInputStyle:style = DCC_TEXT_INPUT_SHORT, const placeholder[] = "", bool:required = true, min_length = 0, max_length = 4000, const value[] = "");
DCC_ShowModal(DCC_Interaction:interaction, DCC_Modal:modal);

forward DCC_OnButtonInteraction(DCC_Interaction:interaction, const custom_id[]);
forward DCC_OnSelectInteraction(DCC_Interaction:interaction, const custom_id[]);
forward DCC_OnModalSubmit(DCC_Interaction:interaction, const custom_id[]);
```

> API names/signatures may change while implementation is being completed.

## Base project

Based on the MIT-licensed `maddinat0r/samp-discord-connector`. The upstream v0.3.6 added command interactions and open.mp support; this project extends that interaction layer to Discord message components and modals.

## Status

Initial development.
