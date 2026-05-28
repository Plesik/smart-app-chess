import { createAssistant, createSmartappDebugger } from '@salutejs/client';

const FRONTEND_COMMAND_TYPES = new Set([
  'create_game',
  'make_move',
  'resign_game',
  'delete_game',
  'refresh_games',
  'change_user_name',
]);

function extractSmartAppData(command) {
  if (!command) {
    return null;
  }

  if (command.type === 'smart_app_data') {
    return command.smart_app_data || command.action || command.payload || null;
  }

  if (command.command?.type === 'smart_app_data') {
    return (
      command.command.smart_app_data ||
      command.command.action ||
      command.command.payload ||
      null
    );
  }

  if (command.action?.type === 'smart_app_data') {
    return command.action.smart_app_data || command.action.payload || null;
  }

  if (command.payload?.type === 'smart_app_data') {
    return command.payload.smart_app_data || command.payload.payload || null;
  }

  if (command.smart_app_data?.type) {
    return command.smart_app_data;
  }

  if (command.smartAppData?.type) {
    return command.smartAppData;
  }

  if (FRONTEND_COMMAND_TYPES.has(command.type)) {
    return command;
  }

  if (FRONTEND_COMMAND_TYPES.has(command.action?.type)) {
    return command.action;
  }

  if (FRONTEND_COMMAND_TYPES.has(command.payload?.type)) {
    return command.payload;
  }

  return null;
}

function toFrontendCommand(smartAppData) {
  if (!smartAppData?.type) {
    return null;
  }

  if (!FRONTEND_COMMAND_TYPES.has(smartAppData.type)) {
    return null;
  }

  return {
    type: smartAppData.type,
    source: 'sber_voice',
    payload: smartAppData.payload || {},
  };
}

export function initSberAssistant({
  getState,
  getRecoveryState,
  onCommand,
  onNavigation,
}) {
  try {
    const token = import.meta.env.VITE_SMARTAPP_TOKEN;
    const initPhrase =
      import.meta.env.VITE_SMARTAPP_INIT_PHRASE || 'Запусти голосовые шахматы';

    const assistant =
      import.meta.env.DEV && token
        ? createSmartappDebugger({
            token,
            initPhrase,
            getState,
            getRecoveryState,
            surface: 'SBERBOX',
            nativePanel: {
              defaultText: 'Скажите команду',
              screenshotMode: false,
              tabIndex: -1,
            },
          })
        : createAssistant({
            getState,
            getRecoveryState,
          });

    assistant.on('data', (command) => {
      if (command?.type === 'navigation') {
        onNavigation?.(command.navigation);
        return;
      }

      const smartAppData = extractSmartAppData(command);
      const frontendCommand = toFrontendCommand(smartAppData);

      if (frontendCommand) {
        onCommand(frontendCommand);
      }
    });

    assistant.on('error', (error) => {
      console.warn('Sber Assistant error:', error);
    });

    return assistant;
  } catch (error) {
    console.warn('Sber Assistant was not initialized:', error);
    return null;
  }
}