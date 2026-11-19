# Omnigram

Omnigram - форк официального клиента Telegram Desktop на C++20 и Qt 6, переносящий интерфейс, физику движения и оформление Telegram для macOS на Linux и Windows.

Проект сохраняет совместимость с кодовой базой Telegram Desktop и добавляет гибкую систему переключения стилей, кинетическую прокрутку со сглаживанием, снятые ограничения клиента и инструменты для работы с несколькими аккаунтами.

---

## Режимы интерфейса

Omnigram поддерживает раздельное переключение внешнего вида через меню "Настройки" -> "Omnigram":

- **macOS Mode**: полное воспроизведение геометрии macOS-клиента. Скруглённые блоки сообщений (радиусы 16pt и 8pt), боковой стеклянный док папок 28pt, поле ввода в форме капсулы 20pt со световым бликом, нативные шрифтовые пропорции и отступы списка чатов.
- **Classic Mode**: стандартный вид Telegram Desktop без изменений разметки.
- **Custom Mode**: выборочное включение элементов. Доступно раздельное управление формой сообщений, боковой панелью, строкой ввода и физикой анимаций.

Для быстрой настройки предусмотрен мастер-переключатель "Всё", связанный со всеми подпунктами через двустороннюю реактивную синхронизацию.

---

## Физика и геометрия

Все визуальные параметры перенесены из исходного кода официального клиента для macOS:

| Элемент | Параметры в Omnigram | Поведение в официальном TDesktop |
| :--- | :--- | :--- |
| **Скругление сообщений** | 16pt для одиночных, 8pt для сгруппированных сообщений, эллиптический вырез хвоста | 10pt / 4pt, треугольный хвост |
| **Поле ввода** | Капсула 20pt Liquid Glass с верхним бликом и объёмным градиентом | Прямоугольный блок с фиксированным фоном |
| **Боковой док папок** | Концентрический радиус 28pt, полупрозрачная подложка, внешняя тень | Прямоугольная панель без скруглений |
| **Список чатов** | Строки 70pt, круглые аватары 50pt, овальные индикаторы непрочитанных | Строки 62pt, аватары 44pt |
| **Кинетика скролла** | Плавное затухание 220 мс по кривой Apple cubic-bezier (0.23, 1.0, 0.32, 1.0) | Фиксированный скачок на шаг колеса мыши |
| **Переходы экранов** | Сдвиг с параллаксом (смещение на треть ширины окна) за 0.4 секунды | Мгновенная смена кадров |

---

## Возможности клиента

В клиенте расширены базовые ограничения и добавлены инструменты управления сессиями:

- **Мультиаккаунт без ограничений**: поддержка до 100 одновременно подключённых профилей вместо трёх по умолчанию.
- **Карточки профиля**: отображение числового Peer ID и номера датацентра (DC1-DC5) непосредственно под именем пользователя, в группах и каналах.
- **Точное время**: секунды в заголовках и сообщениях (формат `hh:mm:ss`).
- **Снятие лимитов**: до 100 закреплённых чатов в корневом списке, до 500 чатов внутри папок, до 100 тем в форумах и до 100 папок.
- **Блокировка рекламы**: полное скрытие спонсорских сообщений в каналах и ботах.
- **Поиск по подаркам**: фильтрация каталога звёздных и коллекционных подарков прямо в окне отправки по названию, номеру токена и стоимости.
- **Zero Telemetry**: отключение Crashpad и Google Breakpad на этапе сборки проекта. Клиент не пишет дампы на диск и не обращается к внешним серверам аналитики.

---

## План развития

- **Массовое выделение чатов**: выбор нескольких диалогов в общем списке чатов (через Shift или режим выбора) для пакетного прочтения, отключения звука, отправки в архив или удаления.
- **Локальные закрепления сообщений**: закрепление важных сообщений в любых чатах, группах и каналах только для себя, даже без прав администратора.
- **Система плагинов**: запуск пользовательских скриптов на TypeScript через изолированную среду WebAssembly (WASM) для кастомизации логики клиента.
- **Стиль Material Design 3**: альтернативная тема оформления на базе Material You со скруглёнными контейнерами, адаптивной цветовой палитрой и плавающими элементами.
- **Движок тем на YAML**: читаемый формат описания стилей с мгновенным применением изменений без перезапуска программы.
- **Менеджер сессий tdata**: перенос и открытие сессий перетаскиванием папки `tdata` или ZIP-архива прямо в окно добавления аккаунта.
- **Очистка устаревшего кода**: удаление невостребованного модуля Telegram Passport для уменьшения объёма исполняемого файла и ускорения сборки.

---

## Сборка из исходников

Для сборки требуется компилятор с поддержкой C++20 (GCC 11+ или Clang 14+), CMake 3.25+, Python 3.10+ и Ninja.

### Linux

Сборка выполняется через изолированное Docker-окружение:

```bash
git clone --recursive https://github.com/DevPulses/omnigram.git
cd omnigram/omni
docker build -t omnigram-builder Telegram/build/docker/centos_env
docker run --rm -v $(pwd):/usr/src/tdesktop omnigram-builder
```

Готовый исполняемый файл появится в каталоге `out/Release/Telegram`.

### Настройка CMake вручную

При прямой сборке без контейнера параметры передаются через флаги CMake:

```bash
cmake -B build -G Ninja \
  -D CMAKE_BUILD_TYPE=Release \
  -D DESKTOP_APP_DISABLE_CRASH_REPORTS=ON \
  -D TDESKTOP_API_TEST=ON
cmake --build build
```

---

# English Description

Omnigram is a Telegram Desktop fork built on C++20 and Qt 6 that brings the interface, motion physics, and visual aesthetics of Telegram for macOS to Linux and Windows.

## Key Highlights

- **macOS Mode**: native Telegram for macOS bubble shapes (16pt / 8pt radii), 28pt glass folder dock, and 20pt Liquid Glass input capsule.
- **Kinetic Physics**: 120 FPS inertial scroll decay based on Apple cubic-bezier curves (220 ms glide) replacing rigid wheel steps.
- **Unlimited Accounts**: connect and switch up to 100 profiles simultaneously.
- **Peer & Datacenter Info**: exact Peer ID and DC indicators displayed across profiles, channels, and groups.
- **Exact Timestamps**: full `hh:mm:ss` precision formatting for messages.
- **Gift Search & Filter**: search bar in the star gifts showcase filtering items by title, NFT number, and price.
- **No Ads & Zero Telemetry**: complete suppression of sponsored messages, with Crashpad and Breakpad removed at build time.

## Roadmap

- **Batch Chat Selection**: select multiple chats in the dialog list (via Shift or multi-select mode) to mark as read, mute, archive, or delete in bulk.
- **Local Pinned Messages**: pin messages in any chat, group, or channel locally for yourself, even without administrator rights.
- **Plugin Engine**: runtime execution of custom TypeScript modules sandboxed with WebAssembly (WASM).
- **Material Design 3**: optional Material You layout layer with dynamic color extraction and expressive rounded containers.
- **YAML Theme System**: human-readable theme format with hot reloading support.
- **tdata Session Loader**: import sessions by dragging and dropping `tdata` directories or ZIP archives.
- **Legacy Cleanup**: removal of unused Telegram Passport components to reduce binary size and compilation time.

## License

Omnigram source code is licensed under GPLv3 with OpenSSL exceptions.
