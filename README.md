## Задача
Написать микросервис на С++ для выдачи рейтинга трейдеров. От ядра системы в микросервис приходят следующие сообщения - user_registered(id,name), user_renamed(id,name), user_deal_won(id,time,amount), user_connected(id), user_disconnected(id).

Способ доставки(через сеть, файл, pipe, и т.д.) сообщений находится вне рамок текущего задания, выбор остается за Вами.

Сервис раз в минуту (и при user_connected) должен отсылать сообщение с содержимым рейтинга для конкретного пользователя: первые 10 позиций рейтинга, позицию юзера в рейтинге, +- 10 соседей по рейтингу для текущего пользователя. Рейтинг сортируется по обороту выигрышных сделок за календарную неделю( с 00:00 понедельника текущей недели).

Требования - сервис должен обеспечивать максимальную производительность для функционирования в high-load режиме.

Реализация сохранения состояния при рестарте микросервиса не входит в задание.

## Архитектура

Есть главный тред, который работает с сетью, сигналами и таймерами. Сообщения в этом треде почти никак не обрабатываются и передаются дальше в остальные треды. В свою очередь при отправке рейтинга сообщения из этих тредов сначала передаются в главный, который и осуществляет отправку.

### Классы

Изпользовалась техника dependency injection. В частноти для mock-тестов.

- Factory - создает все остальные объекты.
- Manager - содержит в себе (прямо или через посредника) все остальные классы, управляет общей логикой, передает задачи в остальные треды Worker'ам, принимает сообщения от них, которые отправляет в Service. Контролирует исполнение graceful shutdown.
- Worker - редактирует данные в Data, подготоваливает сообщения и передает их в Manager.
- Data - содержит даннные клиентов.
- Service - принимает и отправляет сообщения, обрабатывает сигналы (SIGINT, SIGTERM), содержит таймеры и передает их сигналы в Manager.
- AsioService, AsioAcceptor, AsioSocket, AsioTimer, AsioSignals - обертки над boost::asio примитивами для mock-тестов.

### Graceful shutdown
При завершении работы сервиса последовательно отменяются pending асинхронные операции, ожидается выполнение текущих обработчиков, ожидается штатное завершение всех тредов.

### Асинхронная модель
При разработке архитектуры наиболее часто выполняемой операцией считалось увеличение сумм сделок пользователей. Поэтому одна из основных решаемых проблем - ускорение этой операции. Кроме того, акцента при оптимизации требует и распределение заданий в разные треды. Рассматривалось решение с atomic на каждую сумму и потокобезопасной очередью (boost) для раздачи заданий. Но на синтетических тестах такая очередь показала себя хуже варианта с использованием boost::asio::io_service и соответсвующего фунционала для асинхронного выполнения задач с помощью io_service::post. 

Эту модель также удобно использовать и для асинхронной работы с сетью, таймерами и сигналами, которые необходимы для выполнения этого задания.
Таким образом, на каждый тред приходится свой объект io_service, и при необходимости передать какую-либо задачу из одного треда в другой, используется io_service::post.

### Обмен сообщениями
При разработке архитектуры считалось, что объем входящих данных намного превосходит объем исходящих. Поэтому при получении сообщений было решено не использовать std::function (т.к. std::function имплементирован с использованием type erasure, который требует работы с heap). Вместо этого используется специальный класс Callback. Для оптимизации передачи сообщений в io_service::post также было решено отказаться от std::function, вместо этого используется класс Task. 

Рассматривался вариант с boost::variant, который позволяет не использовать heap, если у используемых типов имеется noexept copy constructor. Но такое условие было выполнить невозможно, поскольку требовалась передача объектов, для которых это условие не выдерживается (promise, future).

Многие сообщения передаются с помощью shared_ptr. При этом атомарная операция изменения значения количества ссылок происходит только внутри io_service::post. В остальных случаях используется std::move.

Несмотря на то, что, благодаря использованию бинарного формата передачи данных, объем среднего сообщения довольно мал и при указанной структуре сообщений выигрыш по производительности по сравнению с обычной копией может быть довольно мал, все равно используются указатели и динамическая память в расчете на то, что в реальных приложениях, такие сообщения содержат намного больше информации.

### Потокобезопасность
Для потокобезопасной работы с данными используется способ, который не требует lock'ов и atomic'ов, а вместо этого гарантируется, что у каждого клиента будет только один обрабатывающий тред. Это достигается применением операции % (остаток от деления) к ID клиента и количеству используемых тредов. Так, если ID = 12345, а количество тредов - 4, то сообщения, касающиеся этого клиента всегда будут обрабатываться тредом номер 1 (12345 % 4). При равномерном распределении ID клиентов к значению этого остатка от деления, так же равномерно будут распределены и клиенты между тредами. Нет никаких оснований предполагать другой характер распределения.

Таким образом, все клиенты с ID, кратным количеству тредов, будут обрабатываться тредом номер 0, каждый 1-й по счету - тредом номер 1, каждый 2-й - номер 2 и т.д. Это позволяет отказаться от использования atomic и любых других синхронизирующих примитивов при обработке всех видов сообщений.

### Отправка рейтинга
Еще одна проблема, которую необходимо решить, - это необходимость отправлять рейтинг при коннекте клиентов. Один из вариантов решения - постоянно держать данные в готовом (отсортированном) виде. Но в этом случае очень сложно достичь и скорости, потокобезопасности одновременно. Вместо этого при коннекте клиентов, сообщения с рейтингом могут быть отправлены с некоторой "выравнивающей" задержкой. В текущей конфигурации максимальное значение этой задержки 3 секунды, среднее 1.5. Это значение может быть легко изменено. 

Так каждая минута делится на равные интервалы и при коннекте клиента, данные отправляются в момент ближайшей общей отправки. Например, если коннект клиента произошел в 18:05:05, то рейтинг ему будет отправлен в 18:05:06, то есть при следующем значении секунды кратному 3 (или другому значению по усмотрению).

Для отправки рейтинга используется похожая модель разделения работы между разными тредами. Минута делится на равные трехсекундные интервалы. Получившиеся интервалы распределяются между тредами, используя опервацию % (остаток от деления). Например, если тредов 4, а интервал выбран 3 секунды, то в 18:05:00 отправку рейтинга произведет тред номер 0, в 18:05:03 - тред номер 1, в 18:05:06 - тред номер 2, в 18:05:12 - снова тред номер 0, и т.д.

Каждый тред отправляет рейтинг в общем случае только для "своих" клиентов (ID % номер треда). Чтобы при подключении клиента не ждать с отправкой рейтинга "свой" тред, происходит следующая процедура. 
1. Клиент передаются в "свой" тред.
2. По текущему значению времени определяется ближайший по времени тред, и клиент передается ему (io_service::post). 
3. Осуществляется отправка рейтинга.
4. Клиент возвращается в "свой" тред.
5. Далее рейтинг по нему отправляется штатно раз в минуту.

Как уже говорилось, данные хранятся в неотсортированном виде. Сортировка проводится непосредственно перед отправкой. Процедура отправки следующая:
1. Все треды при старте захватывают свой mutex.
1. От операционной системы раз в три секунды приходит сигнал таймера.
2. По кратности времени определяется следующий тред, который будет отправлять рейтинг "своим" клиентам.
3. Этому треду передается (io_service::post) promise, остальным shared_future.
4. Треды при получении future разблокируют свой mutex и ждут future, после чего снова захватывают свой mutex. Здесь далее треды продолжают работу штатно.
5. Треды при получении promise пытаются захватить mutex'ы других тредов, копируют данные, отпускают mutex'ы других тредов и передют сигнал по promise (именно в таком порядке, потому что свободный mutex захватывать быстрее). Здесь далее треды проводят процедуру подготовки сообщений с рейтингом для всех клиентов, для которых пришло время отправлять рейтинг.

### Работа с данными
Для ускорения копирования данных, чтобы остальные треды ждали минимальное время:
1. Используется vector, что позволяет копировать без необходимости переключаться на разные участки оперативной памяти.
2. Используются элементы, для которых выполняется is_trivial, в случае чего копирование вектора (std::copy) происходит с помощью memmove.

Для быстрого доступа к данным конкретного клиента используется hash (ClientId --> VectorElementId). Hash преаллоцируется на максимально возможной объем, чтобы избежать rehash.

При хранении данных всех клиентов в одном vector'е при доступе к его элементам из разных тредов появляется такая проблема как false sharing. Для борьбы с этим используются разные vector'ы для разных тредов. 

Процедура подготовки рейтинга:
1. Данные сортируются по значению Total.
2. Подготавливается шаблон, состоящий из места для ID клиента и списка из первых десяти (или меньше) клиентов.
3. Для каждого клиента аллоцируется память, копируется шаблон, заполняется значение ID, копируется сам клиент и ближайшие +/-10 (или меньше) его соседей.
4. Сообщение передается в главный тред, где отправляется по сети.

Для ускорения получения и отправки сообщений, получаемые сообщения почти никак не модифицируются, и при последующей отправке почти полностью переиспользуются. Так не требуется тратить ресурсы на дополнительные построения сообщения, вместо этого используется memcpy.

### Учет границ торгового периода
Для реализации требования по вычислению рейтинга только для актуального торгового периода (с предыдущего понедельника) используется таймер.
1. При старте сервиса вычисляются предыдущий и следующий понедельник. Для вычисления используется время unix epoch. 
2. Предыдущий понедельник выставляется в качестве актуального фильтра. Это значение передаются всем тредам, которые в свою очередь сверяют время deal_won с этим пороговым значением.
3. Следующий понедельник используется для выставления таймера. При его срабатывании данные клиентов по сумме сделок дропаются. 
Я не установил причин, по которым нужно поступать иначе, например, сохранять эти данные. Время получаемых сделок также не сохраняется. Сервис хранит только сумму по тем сделкам, которые прошли порог. Как следствие, при отправке это значение проверять дополнительно не требуется. При наступлении нового торгового периода, если время сервиса синхронизировано с временем ядра системы, сделки нового торгового периода не могут прийти раньше, чем сработает локальный таймер. Могут прийти старые, но они отфильтруются согласно новому пороговому значению.

### Структура сообщений
Все сообщения передаются в бинарной форме, что позволяет существенно сократить объем передаваемых данных. Сообщения состоят из header и body.

header:
(Id: 4 bytes)(Type: 4 bytes)

registered:
(header)(NameLength: 1 byte)(Name: NameLength bytes)

renamed:
(header)(NameLength: 1 byte)(Name: NameLength bytes)

deal_won:
(header)(EpochTimeUs: 8 Bytes)(Amount: 8 bytes)

connected:
(header)

disconnected:
(header)

rating:
(header){Top10: (entry): 1..10}(this-client-entry){Plus10: (entry): 1..10}{Minus10: (entry): 1..10}

entry:
(Id: 4 bytes)(Position: 4 bytes)(NameLength: 1 byte)(Name: NameLength bytes)

### Типы
Целые типы хранятся в little-endian as is. При получении значения достаточно каста к нужному целому типу.

Вещественный типа состоит из двух целых чисел: 7 байт на основание и 1 на степень (десяти). Максимальное количество значащих цифр - 16, степень от -127 до +128. 

Строка состоит из целой длины, после которой располагаются байты с char as is.

Для типа сообщений используется 4х байтное целое.

Для избавления от дополнительного построения сообщений при отправке некоторые сообщения переиспользуются:
1. При получении registered из сообщения не извлекаются поля, строка не конвертируется в массив char'ов. Вместо этого сообщение записывается с помощью memcpy без изменения.
2. При получении renamed эта строка полностью перезаписывается без изменения.
3. При отправке рейтинга позиция каждого отдельного клиента получается путем копирования вышеупомятнутого сообщения для этого клиента. В скопированном сообщении в поле с типом сообщения записывается номер в рейтинге.

## Зависимости
- Boost 1.58.0
- Google Test
- Google Mock

## Запуск
1. mkdir \<build-dir>
2. cd \<build-dir>
3. cmake \<source-dir>
4. make
5. cd \<source-dir>
6. chmod +x install.sh
7. chmod +x uninstall.sh
8. ./install.sh \<build-dir> \<port> \<threads-count>
9. sudo systemctl <start|restart|stop|status> RatingService

./uninstall.sh

Можно запускать stand-alone.

## TODO
- Закончить написание тестов. Сейчас почти все fixture и mock готовы. 
- Добавить легко конфигурируемые эмуляторы ядра системы.
- Заменить std::cout, std::cerr на логгеры.
- Известная проблема с быстрым переподключением клиентов.
- Есть возможность для ускорения работы: использовать pool с переиспользуемыми буфферами. Так можно будет избавиться от частых аллокаций/деаллокаций. 

