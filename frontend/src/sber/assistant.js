import { createAssistant, createSmartappDebugger } from '@salutejs/client';

function getSmartAppData(command) {
  if (command?.type !== 'smart_app_data') {
    return null;
  }

  return command.smart_app_data || command.action || null;
}

function toFrontendCommand(smartAppData) {
  if (!smartAppData?.type) {
    return null;
  }

  return {
    type: smartAppData.type,
    source: 'sber_voice',
    payload: smartAppData.payload || {},
  };
}

export function initSberAssistant({ getState, getRecoveryState, onCommand, onNavigation }) {
  let assistant = null;

  try {
    const token = import.meta.env.VITE_SMARTAPP_TOKEN;
    const initPhrase =
      import.meta.env.VITE_SMARTAPP_INIT_PHRASE || 'Запусти голосовые шахматы';

    if (import.meta.env.DEV && token) {
      assistant = createSmartappDebugger({
        token,
        initPhrase,
        getState,
        getRecoveryState,
        surface: 'SBERBOX',
        nativePanel: {
          defaultText: 'Создай партию за белых уровень три',
          screenshotMode: false,
          tabIndex: -1,
        },
      });
    } else {
      assistant = createAssistant({
        getState,
        getRecoveryState,
      });
    }

    assistant.on('data', (command) => {
      if (command?.type === 'navigation') {
        onNavigation?.(command.navigation);
        return;
      }

      const smartAppData = getSmartAppData(command);
      const frontendCommand = toFrontendCommand(smartAppData);

      if (frontendCommand) {
        onCommand(frontendCommand);
      }
    });

    return assistant;
  } catch (error) {
    console.warn('Sber Assistant was not initialized:', error);
    return null;
  }
}