[English](https://github.com/libscanner/universal-reader/blob/main/README.md) | **Русский**

# Библиотека чтения машиночитаемых кодов (unir)

Читает одним вызовом коды разных символик на одном кадре: **DataMatrix** (ECC 200, ГОСТ Р ИСО/МЭК 16022, коды «Честного знака»), **GS1-128** (SSCC на логистических этикетках), **Code 128**, розничные штрихкоды **EAN-13, EAN-8, UPC-A, UPC-E** и **QR** (модель 2, версии 1–40, в том числе GS1 QR).

`unir` — коммерческая библиотека с закрытым исходным кодом: C++20 API, плоский C ABI, пакет для Python и утилита командной строки. Один предел времени на кадр — на все символики, лицензия на все — одна. Необязательная сеть-разведчик (в поставке) сначала находит коды и ускоряет чтение нескольких кодов на кадре.

В этом репозитории — публичные заголовки, примеры, ссылки на документацию и тексты лицензий. **Исходного кода библиотеки здесь нет.** Собранные библиотеки приложены к [выпускам на GitHub](https://github.com/libscanner/universal-reader/releases/latest).

**[Прочитать код со снимка онлайн](https://libscanner.com/scan/)** — бесплатно, в браузере, ничего не нужно устанавливать.

## Платформы

| Платформа | Статическая библиотека C++ + C ABI + утилита | Колесо Python |
|---|---|---|
| Windows x64 | `unir-1.2.2-windows-x64.zip` | `libscanner_unir-1.2.2-py3-none-win_amd64.whl` |
| Linux x86-64 | `unir-1.2.2-linux-x64.tar.gz` | `libscanner_unir-1.2.2-py3-none-manylinux_2_36_x86_64.whl` |
| Linux ARM64 (Raspberry Pi 3/4/5, 64 бит) | `unir-1.2.2-linux-arm64.tar.gz` | `libscanner_unir-1.2.2-py3-none-manylinux_2_36_aarch64.whl` |
| Linux ARMv7 (Raspberry Pi 2/3/4/5, 32 бит) | `unir-1.2.2-linux-armhf.tar.gz` | `libscanner_unir-1.2.2-py3-none-manylinux_2_36_armv7l.whl` |

Для каждой платформы есть и архив только с утилитой `unir` (`…-cli.zip` / `…-cli.tar.gz`).

- Сборки под Windows сделаны MinGW-w64 GCC (MSYS2, оболочка MINGW64). Из MSVC и других компиляторов — через C ABI (`unir_c.dll`).
- Сборки под Linux сделаны GCC 12 на Debian 12 (glibc 2.36): для C++ API нужен GCC 12 или новее; колёсам — glibc 2.36+ (Debian 12 / Raspberry Pi OS Bookworm и новее).
- Python 3.9 и новее, без зависимостей.

## Загрузка

- **Сборки:** [выпуски на GitHub → последний](https://github.com/libscanner/universal-reader/releases/latest) — архивы для каждой платформы и колёса Python.
- **Python:** [PyPI → libscanner-unir](https://pypi.org/project/libscanner-unir/) — `pip install libscanner-unir`, импорт — `unir`.
- **Лицензии:** тарифы на <https://libscanner.com/>. Ключ активации появляется в личном кабинете после покупки.

### Пробный период

Любая сборка работает **30 дней с первого запуска на машине** — без регистрации, без ключа и без обращения к сети. Дальше нужен ключ лицензии с <https://libscanner.com>; без него `scan()` сразу возвращает `Status::Unlicensed` и кадр не обрабатывает.

**Контейнеры (Docker, Kubernetes)** — с версии 1.2 в контейнере без ключа пробного периода нет: библиотека работает по лицензии **на экземпляры**, ключ — в переменной окружения `UNIR_LICENSE_KEY` или секрете `/run/secrets/unir.key`, контейнеры можно пересоздавать. Пробный ключ для контейнеров (2 экземпляра, 30 дней) — в личном кабинете на <https://libscanner.com/>. Подробнее: [лицензия в Docker и Kubernetes](https://libscanner.com/articles/licenziya-docker-kubernetes/).

## Состав поставки

```text
include/unir/{Unir,Geometry,Image,License,Scanner,Settings,Symbology,Version}.h   C++ API
include/unir/unir_c.h              C ABI
lib/libunir.a                      статическая библиотека C++
lib/pkgconfig/unir.pc
lib/libunir_c.so | bin/unir_c.dll  библиотека C ABI (+ lib/unir_c.dll.a под Windows)
lib/pkgconfig/unir_c.pc
bin/unir                           утилита командной строки
share/doc/unir/licenses/           лицензии стороннего кода
```

Заголовки в [`include/`](https://github.com/libscanner/universal-reader/tree/main/include/unir) этого репозитория объявляют тот же API, что заголовки поставки 1.2.2; отличаются только комментарии (здесь — на английском, в поставке — на русском).

## Быстрый старт

### C++

```cpp
#include <unir/Unir.h>
#include <iostream>

int main() {
    unir::Settings settings = unir::Settings::multi(0);   // все коды на кадре
    settings.symbologies = unir::kAllSymbologies;         // или, например, Symbology::DataMatrix | Symbology::Gs1_128

    const unir::Scanner scanner(settings);
    const unir::ScanResult r = scanner.scanFile("label.jpg");  // загрузка входит в срок кадра

    for (const unir::Code& c : r.codes)
        std::cout << unir::nameOf(c.symbology) << ": " << c.text << "\n";
}
```

Статическая компоновка через pkg-config — запрос **обязательно** с `--static`:

```sh
PKG_CONFIG_PATH=/путь/к/unir-1.2.2-linux-x64/lib/pkgconfig pkg-config --static --cflags --libs unir
```

Кадр из своего конвейера (SDK камеры, буфер OpenCV, bitmap) передаётся видом на существующий буфер, без копирования:

```cpp
const unir::ImageView view{ m.data, m.cols, m.rows, (int)m.step, unir::PixelFormat::Bgr8 };
const unir::ScanResult r = scanner.scan(view);
```

Полный пример: [`examples/cpp`](https://github.com/libscanner/universal-reader/tree/main/examples/cpp). Пример на C ABI (MSVC, другие языки): [`examples/c`](https://github.com/libscanner/universal-reader/tree/main/examples/c).

### Python

```sh
pip install libscanner-unir     # PyPI: Windows x64, Linux x86-64 / ARM64 / ARMv7 (glibc 2.36+)
```

```python
import unir

result = unir.scan_file("label.jpg", unir.Settings.multi(0))
for code in result.codes:
    print(code.symbology_name, code.text.decode("utf-8", errors="replace"))
```

`Code.text` — `bytes`: разделитель GS1 остаётся байтом `0x1D`. Колесо ставит и команду `unir`. Пример: [`examples/python`](https://github.com/libscanner/universal-reader/tree/main/examples/python).

### Командная строка

```sh
unir label.jpg
unir label.jpg --codes 0 --sym dm,gs1
unir --folder ./frames
unir --license status --json
```

Другие команды: [`examples/cli`](https://github.com/libscanner/universal-reader/tree/main/examples/cli).

Сообщения — на языке системы: на русской — по-русски, на любой другой — по-английски; ключ `--lang ru|en` или переменная окружения `LIBSCANNER_LANG` задают язык явно.

## Документация

- C++ / C ABI / утилита: <https://libscanner.com/docs/universal-reader/> (English: <https://libscanner.com/en/docs/universal-reader/>)
- Python: <https://libscanner.com/docs/universal-reader/?lang=python> (English: <https://libscanner.com/en/docs/universal-reader/?lang=python>)

## Лицензия

Проприетарная — см. [LICENSE.txt](https://github.com/libscanner/universal-reader/blob/main/LICENSE.txt) (лицензионное соглашение с пользователем; юридическую силу имеет русский текст, [LICENSE.en.txt](https://github.com/libscanner/universal-reader/blob/main/LICENSE.en.txt) — перевод на английский для сведения). Начало использования ПО означает согласие с соглашением.

Сторонние компоненты: [THIRD-PARTY-NOTICES.ru.txt](https://github.com/libscanner/universal-reader/blob/main/THIRD-PARTY-NOTICES.ru.txt) (на английском — [THIRD-PARTY-NOTICES.txt](https://github.com/libscanner/universal-reader/blob/main/THIRD-PARTY-NOTICES.txt)) и тексты лицензий в [`licenses/`](https://github.com/libscanner/universal-reader/tree/main/licenses).

Код в [`examples/`](https://github.com/libscanner/universal-reader/tree/main/examples) — под лицензией MIT-0 ([examples/LICENSE](https://github.com/libscanner/universal-reader/blob/main/examples/LICENSE)): копируйте в свои проекты свободно; сама библиотека — проприетарная (LICENSE.txt).

## Связь

- Почта: [libscanner@yandex.com](mailto:libscanner@yandex.com)
- Форма обратной связи: <https://libscanner.com/feedback>
- Уязвимости: см. [SECURITY.md](https://github.com/libscanner/universal-reader/blob/main/SECURITY.md)

«GS1» — товарный знак GS1 AISBL. «Честный знак» — товарный знак ООО «Оператор-ЦРПТ». Продукт не связан с владельцами этих знаков и ими не сертифицирован; названия упоминаются только для описания поддерживаемых форматов.
