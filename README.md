
# AnTi pipeline

AnTi pipeline  (**An**tenna to **Ti**ming pipeline) --- программа для обработки наблюдательных данных пульсаров. Программа предназначена для сборки интегрального и эталонного профиля импульсов, определения момента прихода импульса данного сеанса наблюдений.

## Описание 

AnTi pipeline имеет два режима работы: определение момента прихода импульса сеанса наблюдений и сборка эталонного профиля импульса пульсара из серии интегральных профилей.

### Конфигурационный файл
Для работы программы необходим конфигурационный файл. Имя конфигурационного  файла передается программе в качестве аргумента. Стандартное значение имени конфигурационного файла `default.cfg`. Конфигурационный файл включает в себя следующие параметры:
* `verbose = [1, 0]` --- режимы вывода информации о работе программы. 1 --- подробный вывод, 0 --- краткий. Стандартное значение --- 1
* `rawdata_dir = [directory_name]`--- директория, содержащая входные данные программы
* `output_dir = [directory_name]` --- директория, содержащая выходные данные программы
* `tplfile  = [file.tpl]` --- имя файла, содержащего эталонный профиль импульса
* `do_filtration = [1, 0]` --- режим фильтрации сигнала при сборке интегрального профиля импульса. 1 --- производить фильтрацию, 0 --- не производить фильтрацию. Стандартное значение --- 1
* `deriv_threshold = [float number]` --- порог чувствительности грубого фильтра. Стандартное значение --- 0.02
* `deriv_width = [int number]` --- ширина срабатывания грубого фильтра. При срабатывании фильтра на одном из частотных каналов, фильтр считается сработавшим на указанном числе каналов по обе стороны от него
* `median_threshold = [float number]` --- порог чувствительности точного фильтра. Стандартное значение --- 1.5
* `median_width = [int number]` --- ширина срабатывания точного фильтра. При срабатывании фильтра на одном из частотных каналов, фильтр считается сработавшим на указанном числе каналов по обе стороны от него
* `get_fr  =	[1, 0]` --- режимы печати АЧХ системы данного сеанса. 1 --- вывести АЧХ до и после фильтрации в файл, 0 --- не выводить АЧХ в файл. Стандартное значение --- 1
* `do_tpl  = [1, 0]` --- режим сборки эталонного профиля импульса. 1 --- собрать эталонный профиль импульса, 0 --- определить момент прихода импульса сеанса наблюдений. Стандартное значение --- 0
* `num_files = [int number]` --- количество файлов, которое необходимо обработать.
* `runs:` --- ключевое слово. Все строки после него будут приняты за имена файлов, которые необходимо обработать в ходе работы программы

Значение параметров может быть модифицировано из командной строки путем указания имени параметра и его нового значения в формате: `--[name]=[new_value]`
При указании параметра, принимающего значения 1 или 0, параметру устанавливается значение 1. Параметр указывается без нового значения: `--[name]`.

### Определение момента прихода импульса
В режиме определения момента прихода импульса AnTi pipeline считывает файлы наблюдений и файл, содержащий эталонный профиль импульса. При необходимости программа находит интегральный профиль импульса. Путем кросс-корреляционной процедуры интегрального и эталонного профилей определяется момент прихода импульса данного сеанса. Момент прихода импульса добавляется в файл `[output_dit]/toa` в формате ITOA.


### Сборка эталонного профиля импульса
В данном режиме программа считывает файлы наблюдений, при необходимости составляет интегральный профиль импульса, и, усредняя интегральные профили, собирает эталонный профиль импульса. Положение итогового профиля в окне наблюдения, ширина окна наблюдения, значение временного дискрета наследуются от первого интегрального профиля, участвующего в обработке.

### Форматы файлов
Программа работает с файлами наблюдений в двоичном формате БСА, формате `.srez` или выходном формате программы `.prf`, аналогичном `.srez`. Формат файла эталонного профиля: `.tpl`, АЧХ системы: `.fr`.  Примеры файлов каждого из форматов находятся по [ссылке](). 

## Запуск программы

### Зависимости

* AnTi pipeline предназначен для работы в ОС Linux
* Для установки программы необходимы утилиты `g++` и `make`

### Установка
```
    git clone git@github.com:StepanRepo/AnTi-pipeline.git
    cd AnTi-pipeline
    git submodule init && git submodule update
    make 
```

### Запуск

* Для работы программы необходимо заполнить конфигурационный файл:
```
	./main [path_to_cfg]
```
*  Или использовать стандартный конфигурационный файл
```
	./main
```

## Краткая информация
Краткая информация о программе пока недоступна.


## Авторы

* [Степан Андрианов](https://t.me/wizard_of_math)

## История версий

* 0.1
    * Первый релиз

## Ссылки
* Написание AnTi pipeline было вдохновлено программой [lpa-timer](https://github.com/praoPulsarTiming/lpa-timer)
* Принцип работы программы описан в статьях [Binary and Millisecond Pulsars](https://link.springer.com/article/10.12942/lrr-2005-7) и [Пульсарная шкала времени](chrome-extension://bpnedgjmphmmdgecmklcopblfcbhpefm/viewer.html?url=http%3A%2F%2Fproceedings.lebedev.ru%2Fwordpress%2Fwp-content%2Fuploads%2F2013%2F06%2F1989-199.djvu) (с. 149)
