# Aimware 2016 - Improved & Refactored

> Полностью переработанный лоадер дампа памяти Aimware CS:GO 2016 с современной C++ архитектурой

## Что это?

Оригинальный Aimware 2016 - это internal чит для CS:GO, который хранился как 4 дампа памяти:
- `0x34E10000` (192KB) - Code section
- `0x43AF0000` (90KB) - Data section  
- `0x76ED0000` (110KB) - String resources
- `0x7C4A0000` (122KB) - CRT helpers

Лоадер маппит их в фиксированные адреса, восстанавливает импорты и хукает движок Source.

## Что улучшено?

### Архитектура
- **core/logger.hpp** - цветной thread-safe логгер с уровнями
- **core/memory_manager.hpp** - RAII менеджер фиксированных регионов
- **core/sdk.hpp** - полный SDK с Vector math, QAngle, LagRecord
- **core/config_system.hpp** - система конфигов на `std::filesystem`

### Менеджеры
- **hooking_manager** - переписан с нуля, без `memset(this)`, с move-семантикой, безопасный `GetVTCount`
- **netvars_manager** - исправлена рекурсия, `optional` паттерны, `DumpNetvars`
- **util.hpp** - современный `PatternScanner` с `optional<uintptr_t>`

### Движки (decompiled)
Все заглушки заменены на реальные реализации:

| Движок | Что сделано |
|--------|-------------|
| **aimbot** | FOV расчет, hitchance с 256 сидами (mt19937), multipoint, autowall, movement fix |
| **antiaim** | Pitch/Yaw enum, spinbot, jitter, fakelag с LBY break, at-targets |
| **resolver** | 5 режимов (LBY, bruteforce, anim, advanced), история LBY, missed shots |
| **visuals** | ESP box/health/name/weapon/skeleton, chams, NoSmoke, LockCursor unlock |
| **skinchanger** | XOR decrypt строк, paintkit override, glove changer, force update |
| **config_ui** | INI парсер, санитайзинг, DrawCheckbox/Slider/Combobox заготовки |

### Инжектор
- RAII `HandleWrapper`
- Проверка `VirtualQueryEx` перед аллокацией
- Ожидание remote thread с timeout
- Проверка уже запущенного CS:GO
- Аргументы: `--exe`, `--dll`, `--params`, `--no-inject`

### Проект
- Добавлены Debug/Release конфигурации
- Исправлен `Optimization Disabled + WholeProgramOptimization true` конфликт
- `SubSystem Windows` вместо Console для DLL
- `AdditionalIncludeDirectories` для core/

## Структура

```
aimware2016/
├── core/
│   ├── logger.hpp          # Логирование
│   ├── memory_manager.hpp  # Фиксированные регионы
│   ├── sdk.hpp             # SDK + math
│   └── config_system.hpp   # Конфиги
├── decompiled/
│   ├── decompiled_sdk.hpp      # Базовый SDK для движков
│   ├── aimbot_engine.*         # Аимбот
│   ├── antiaim_engine.*        # Анти-аим
│   ├── resolver_engine.*       # Резолвер
│   ├── visuals_engine.*        # ESP/Chams
│   ├── skinchanger_engine.*    # Скины
│   ├── config_ui_engine.*      # Меню
│   └── aimware_decompiled.hpp  # Инициализация
├── aw.h / aw.cpp               # Главный лоадер (переписан)
├── hooking_manager.*           # VMT хуки (переписан)
├── netvars_manager.*           # Netvars (переписан)
├── util.hpp / util.h           # Паттерн сканер
└── b*.h                        # Бинарные дампы

awinjector/
└── main.cpp                    # Инжектор (переписан)
```

## Сборка

**Требования:**
- Visual Studio 2022, v143, Win32
- Windows SDK 10.0.22621.0

**Сборка:**
```
1. Открыть aimware2016.sln
2. Выбрать Rel_Mar2017|x86 или Rel_Mar2018|x86
3. Собрать -> out\Rel_Mar2017\aimware.dll + out\injector.exe
```

## Запуск

```cmd
# Автоматический запуск + инжект
injector.exe --exe "C:\Steam\...\csgo.exe" --dll ".\out\Rel_Mar2017\aimware.dll"

# Только инжект в уже запущенную игру
injector.exe --dll ".\out\Rel_Mar2017\aimware.dll"

# Кастомные параметры
injector.exe --params "-insecure -steam -novid -windowed"
```

## Документация

- `REVERSE_ENGINEERING.md` - оригинальный реверс-отчет (адреса, импорты, дизасм)
- `ANALYSIS_REPORT.md` - полный анализ проблем и что исправлено (RU)

## TODO

- [ ] Полный SDK (IClientEntity с методами)
- [ ] WorldToScreen через ViewMatrix
- [ ] Autowall с penetration
- [ ] ImGui меню
- [ ] JSON конфиги
- [ ] x64 поддержка

## Лицензия

Образовательный проект. Используйте на свой страх и риск. Автор не несет ответственности за баны.

## Кредиты

- Оригинальный Aimware 2016 binary dump
- Рефакторинг и доработка - Arena Agent Mode
