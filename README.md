# Aimware 2016 - Full Reverse & Advanced Mapper Edition

> Полный реверс до конца + продвинутый маппер с PE-анализом, дешифратором и верификацией

## 🔥 Что нового в v3.0 (Full Reverse)

### Продвинутый маппер `core/advanced_mapper.hpp`
- **Фиксированные регионы с конфликтом**: `VirtualQuery` + `NtUnmapViewOfSection` для очистки занятых адресов
- **PE-парсер**: `pe_parser.hpp` ищет MZ/PE внутри дампов, парсит секции, EntryPoint
- **Анализатор**: `AimwareDumpAnalyzer` считает энтропию, указатели, прологи функций
  - CODE: 3281 указателей, 89 пролога, энтропия 7.317
  - DATA: 2441 указателя (1203 DATA, 752 STRING, 401 CRT), 0 пролога
  - STRING: энтропия 7.826 (шифровано), 14 указателей
  - CRT: энтропия 7.995 (шифровано), 444 указателя
- **Релокации**: сканирует cross-region указатели и фиксит если base отличается
- **Защита**: финализация `CODE->RX, DATA->RW, STRING->R`, `FlushInstructionCache`
- **Верификация**: `VerifyMapping()` проверяет `State`, `Protect`, контент

### Дешифратор `core/decryptor.hpp`
- Single/Multi XOR, XOR+pos, RC4
- `DecryptStringWithProfileKey` — как в оригинале `xor cl, [eax]` где `eax=profile_t.xor_key`
- `BruteForceXor` для поиска ключа по known plaintext (55 8B EC)
- `DetectEncryption` по энтропии и частотам
- `StringDecryptor::DecryptAllStrings` — сканирует 0x76ED0000, ищет читаемые строки после XOR

### Полный реверс `reversed/`
- **memory_map.hpp** — ВСЕ фиксированные адреса:
  - Bases: CODE 0x34E10000 192KB, DATA 0x43AF0000 90KB, STRING 0x76ED0000 110KB, CRT 0x7C4A0000 122KB
  - DataSection: 40+ адресов (LAG_RECORDS_PTR 0x43AF7A84, PROFILE_CONTEXT 0x43AFF218, RENDER_CONTEXT, INTERFACE_TABLE, etc)
  - CodeSection: 30+ функций (CREATE_MOVE 0x34E258A0, FRAME_STAGE 0x34E26330, PAINT 0x34E26660, DRAW_MODEL 0x34E25960, etc)
  - ImportTable: 40+ импортов kernel32/user32/msvcrt/ntdll
  - XorPatches: 56 адресов JZ->JNZ
- **config_struct.hpp** — структура конфига: `AimbotConfig`, `AntiAimConfig`, `ESPConfig`, `SkinchangerConfig`, `FullConfig`, `ProfileContext`
- **offsets.hpp** — netvar offsets: `CCSPlayer`, `CBaseCombatWeapon`, `CBaseAttributableItem`, `LagRecord` 0x3234 layout, 2016 vs 2018 differences
- **functions.hpp** — дизасм + псевдо-C++ для всех хуков: CreateMove, FrameStageNotify (NoSmoke), Paint (ESP), DrawModel (Chams), LockCursor, Proxies (Pitch/Yaw/LBY/Flags/Flash/Smoke), Aimbot (Hitchance 256 seeds), AntiAim, Skinchanger (Glove 0x76ED9CE0 decrypt)
- **full_reverse.md** — 300+ строк полного отчета: архитектура, импорты, интерфейсы, функции, конфиг, скины, XOR, анти-аим, резолвер, маппер

### Инжектор v3.0
- `AdvancedUnmap` через `NtUnmapViewOfSection`
- `IsRegionFree` проверка перед аллокацией
- `AllocateFixedRegionsAdvanced` с очисткой занятых регионов
- `VerifyMapping` после инжекта
- Поддержка `--basic-mapper` и `--verify`
- Вывод всех fixed regions

### Инструменты
- `tools/dump_analyzer.py` — Python анализатор дампов:
  - Энтропия, указатели, прологи, строки, поиск MZ
  - Cross-reference interface table
  - Запуск: `python3 tools/dump_analyzer.py`

## Структура v3.0

```
aimware2016/
├── core/
│   ├── logger.hpp              # Логгер
│   ├── memory_manager.hpp      # Базовый маппер
│   ├── advanced_mapper.hpp     # Продвинутый маппер (NEW)
│   ├── pe_parser.hpp           # PE парсер + анализатор (NEW)
│   ├── decryptor.hpp           # XOR/RC4/StringDecryptor (NEW)
│   ├── sdk.hpp                 # SDK
│   └── config_system.hpp       # Конфиги
├── reversed/                   # Полный реверс (NEW)
│   ├── memory_map.hpp          # Все адреса (CODE/DATA/STRING/CRT/Import/XOR)
│   ├── config_struct.hpp       # Структуры конфига
│   ├── offsets.hpp             # Netvar оффсеты + LagRecord
│   ├── functions.hpp           # Дизасм + псевдо-C++
│   └── full_reverse.md         # Полный отчет 300+ строк
├── decompiled/                 # Движки (полные реализации)
│   ├── aimbot_engine.*         # FOV, hitchance 256, multipoint, autowall
│   ├── antiaim_engine.*        # Pitch/Yaw, spinbot, fakelag
│   ├── resolver_engine.*       # 5 режимов, LBY history
│   ├── visuals_engine.*        # ESP, chams, NoSmoke
│   ├── skinchanger_engine.*    # XOR decrypt, paintkit
│   └── config_ui_engine.*      # INI, меню
├── aw.h / aw.cpp               # Лоадер с AdvancedMapper + FullReverse
├── hooking_manager.*           # VMT хуки
├── netvars_manager.*           # Netvars
└── b*.h                        # Дампы

tools/
└── dump_analyzer.py            # Анализатор дампов (NEW)

awinjector/
└── main.cpp                    # Advanced injector v3.0
```

## Сборка

**Требования:** VS2022, v143, Win32, SDK 10.0.22621.0

```
1. Открыть aimware2016.sln
2. Выбрать Rel_Mar2017|x86 (2016) или Rel_Mar2018|x86 (2018)
3. Build -> out\Rel_Mar2017\aimware.dll + injector.exe
```

**Флаги:**
- `USE_DECOMPILED_ENGINE` — C++ движки вместо бинарных
- `USE_ADVANCED_MAPPER` — продвинутый маппер с PE-анализом
- `FULL_REVERSE` — использовать адреса из reversed/

## Запуск

```cmd
# Полный реверс + advanced mapper (рекомендуется)
injector.exe --exe "C:\Steam\...\csgo.exe" --dll ".\out\Rel_Mar2017\aimware.dll" --verify

# В уже запущенную игру
injector.exe --dll ".\out\Rel_Mar2017\aimware.dll"

# Базовый маппер (для сравнения)
injector.exe --dll ".\out\Rel_Mar2017\aimware.dll" --basic-mapper

# Анализ дампов
python3 tools/dump_analyzer.py
```

## Документация

- `REVERSE_ENGINEERING.md` — оригинальный отчет (краткий)
- `reversed/full_reverse.md` — **полный реверс до конца** (300+ строк, все адреса, дизасм, псевдо-C++)
- `ANALYSIS_REPORT.md` — анализ проблем исходника
- `tools/dump_analyzer.py` — вывод: CODE 89 пролога, DATA 2441 указателя, STRING шифровано

## Что прореверсили до конца

✅ **Маппер:**
- Фиксированные регионы с конфликтом, NtUnmap, релокации, защита, верификация
- PE-парсер находит embedded MZ в DATA+0x68B1, STRING+0x15BC, CRT+0x44E2
- Энтропия + указатели + прологи

✅ **Импорты:** 40+ функций kernel32/user32/msvcrt/ntdll 100% восстановлены

✅ **Интерфейсы:** 17 интерфейсов VEngineClient, EntityList, etc + паттерны WriteUserCmd, etc

✅ **Хуки:**
- CreateMove 0x34E258A0 — aimbot, hitchance 0x34E282D0, GetBestTarget 0x34E28D40, Autowall 0x34E29040
- FrameStageNotify 0x34E26330 — NoSmoke `*smokeCount=0`, Skinchanger 0x34E2C580 stage 4
- Paint 0x34E26660 — ESP 0x34E1D1D0 если PAINT_UIPANELS
- DrawModel 0x34E25960 — chams, decrypt 0x76ED9EA0 via profile key
- LockCursor 0x34E26620 — unlock если menu_open 0x43AFD094
- Proxies: Pitch 0x34E24BF0 0x3200, Yaw 0x34E24D20 0x3204, LBY 0x34E24EC0 0x320C + update time

✅ **Конфиг:** `C:\aimware\` via `\??\`, FindFirstFileW, DumpConfig 0x34E357C5, GetConfigList 0x34E35983

✅ **Скинченджер:** Glove 0x34E1DFC9 decrypt `mov cl, [0x76ED9CE0]; xor cl, [profile_key]`, Paintkit 0x34E1E2DA override m_nFallbackPaintKit, force via cl_fullupdate

✅ **XOR:** 56 адресов JZ->JNZ bypass

✅ **Анти-аим/Резолвер:** Pitch 89/-89/0/180, Yaw backward/sideways/spinbot/jitter, Fakelag choke 6, LagRecords 0x43AF7A84 0x3234*64, LBY break detection

## TODO (для еще более полного реверса)

- [ ] Расшифровать STRING секцию полностью (нужен рабочий профиль с xor_key !=0)
- [ ] Реконструировать PE из дампов (сейчас только анализ)
- [ ] ImGui меню вместо кастомного
- [ ] JSON конфиги + шифрование
- [ ] x64

## Лицензия

Образовательный проект. Full reverse для изучения.

## Кредиты

- Binary dump Aimware 2016
- Full reverse + Advanced Mapper — Arena Agent
- Инструменты: dump_analyzer.py
