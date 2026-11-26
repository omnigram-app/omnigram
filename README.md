# Omnigram

**Omnigram** — функциональный форк Telegram Desktop на C++20 и Qt 6 с упором на приватность, снятие клиентских ограничений, инструменты для стримеров и расширяемость через плагины, сохраняющий чистый и привычный интерфейс оригинального клиента.

---

## Возможности клиента

### Приватность и Streamer Mode
- **Гибкий Streamer Mode**: защита от деанонимизации на стримах и скриншотах с 4 независимыми группами правил — **Всем (Everyone)**, **Себе (Self)**, **Контактам (Contacts)** и **Выбранным (Selected)**, с поддержкой списков исключений для каждой группы.
- **Выборочное скрытие данных**: раздельное скрытие `@username`, числового `Peer ID`, номера телефона, отображаемого имени и аватара (с заменой на любое локальное изображение).
- **Кастомная замена и защита от фейков**: любой свой текст замены на любом языке (по умолчанию `*`) с автоматической плашкой `[SM]` для предотвращения поддельных скриншотов.

### Скорость и снятие ограничений
- **Быстрая загрузка и выгрузка (Fast Download & Upload)**: мгновенный старт с 8 параллельных соединений к датацентру и разгон до 16 потоков с увеличенным окном чанков (настраивается в `Omnigram Settings`).
- **Мультиаккаунт без ограничений**: поддержка до 100 одновременно подключённых аккаунтов вместо стандартных трёх.
- **Снятие клиентских лимитов**: расширены лимиты на закреплённые чаты, папки и темы (включается в настройках).
- **Блокировка рекламы (No Ads)**: полное скрытие спонсорских сообщений в каналах.

### Удобство и Power-User фичи
- **Точное время сообщений**: отображение времени с точностью до секунд (`hh:mm:ss`).
- **Инфо-карточки профиля**: отображение числового `Peer ID` и датацентра (`DC1–DC5`) в профилях пользователей, групп и каналов.
- **Контроль панели эмодзи**: открытие панели эмодзи и стикеров только по клику (отключение всплытия при наведении).
- **Фикс `<3`**: отключена принудительная автозамена `<3` на графическое сердце.
- **Плавный скролл**: кинетическая прокрутка в `lib_ui` (`patches/lib_ui_smooth_scroll.patch`).

### Изоляция, плагины и сборка
- **Изолированные данные**: клиент хранит сессии и настройки в отдельной директории `OmniGram` (`~/.OmniGram/` на Linux), не конфликтуя с официальным Telegram Desktop.
- **Поддержка AppImage**: автоматическая упаковка Linux-сборки в автономный `OmniGram-x86_64.AppImage`.
- **Система плагинов (TypeScript / JS)**: встроенный менеджер плагинов (`PluginManager`), открытие папки плагинов и горячая перезагрузка из настроек.
- **Zero Telemetry**: отключение краш-репортов (`Crashpad` / `Breakpad`) на этапе сборки.

---

## План развития (Roadmap)

- **Полноценный рантайм плагинов (TS + WASM)**: настройки плагинов в отдельном окне и вкладках, расширенный SDK.
- **Поиск и фильтрация подарков**: поиск по каталогу подарков и коллекционок (название, номер, цена в Stars).
- **Менеджер сессий `tdata`**: импорт и экспорт аккаунтов перетаскиванием папки `tdata` или ZIP-архива.
- **Программа раннего комьюнити и саппортеров**: закрытый чат, ранние бета-сборки и статус `Early Supporter`.
- **Собственный MTProto-сервер (долгосрочная цель)**: альтернативная сеть с ботом-регистратором, выбором сервера на экране входа и уникальными коллекционными подарками.

---

## Сборка из исходников

Для сборки требуется компилятор с поддержкой C++20, CMake 3.25+, Python 3.10+ и Ninja.

### Linux (Docker)

```bash
git clone --recursive https://github.com/omnigram-app/omnigram.git
cd omnigram
docker build -t omnigram-builder Telegram/build/docker/centos_env
docker run --rm -v $(pwd):/usr/src/tdesktop omnigram-builder
```

Готовый бинарник появится в `out/Release/Telegram`. Для упаковки в AppImage используется скрипт `./build_appimage/package.sh`.

### Ручная настройка CMake

```bash
cmake -B build -G Ninja \
  -D CMAKE_BUILD_TYPE=Release \
  -D DESKTOP_APP_DISABLE_CRASH_REPORTS=ON \
  -D TDESKTOP_API_TEST=ON
cmake --build build
```

---

# English Description

**Omnigram** is a power-user fork of Telegram Desktop built on C++20 and Qt 6, focused on privacy, lifted client-side limits, streamer protection, fast file transfers, and plugin extensibility while keeping the clean native desktop UI.

## Key Highlights

- **Flexible Streamer Mode**: 4 independent target scopes (`Everyone`, `Self`, `Contacts`, `Selected`) with per-scope exceptions. Mask `@username`, `Peer ID`, phone number, display name, and avatar (with a custom local image). Supports custom replacement text with an automatic `[SM]` anti-fake badge.
- **Fast Download & Upload**: starts with 8 parallel connections per DC and scales up to 16 parallel connections with larger in-flight chunk windows (toggleable in settings).
- **Unlimited Accounts**: connect and switch up to 100 accounts simultaneously.
- **Peer & Datacenter Info**: exact bare `Peer ID` and DC indicator (`DC1–DC5`) shown directly in user, group, and channel profiles.
- **Exact Timestamps**: full `hh:mm:ss` precision formatting for messages.
- **Lifted Client Limits & No Ads**: expanded local limits for pinned chats/folders and suppression of sponsored messages.
- **Click-Only Emoji Panel & `<3` Fix**: disable hover popup on the emoji/stickers panel and keep `<3` as plain text.
- **Isolated Storage & AppImage**: uses a dedicated `OmniGram` data directory (`~/.OmniGram/` on Linux) alongside official Telegram Desktop, with `OmniGram-x86_64.AppImage` packaging.
- **Plugin Infrastructure (TS / JS)** & **Zero Telemetry**: built-in `PluginManager` UI and crash reporting disabled at build time.

## License

Omnigram source code is licensed under GPLv3 with OpenSSL exceptions.
