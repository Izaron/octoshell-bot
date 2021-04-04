Чтобы построить образ по Dockerfile, надо запустить эту команду:
```
docker build -t docker-octoshellbot .
```
В первый раз строиться образ будет довольно долго (несколько минут), но в дальнейшем, при повторных запусках, будет только перестраиваться исходники OctoshellBot (остальные слои будут закэшированы).

Эту команду надо запускать всегда, когда вы что-то поменяли в коде или других файлах внутри `src`.


Построенный образ можно запустить внутри контейнера:
```
docker run --publish 13000:13080 --detach --name octocontainer docker-octoshellbot
```
`13000` это порт, который надо "открыть", то есть сделать возможным обращение по этому порту. Это порт HTTP-сервера приложения.

Чтобы посмотреть логи контейнера, можно запустить команду:
```
docker container logs octocontainer
```

Перед тем, как перезапустить контейнер (когда будет перестроен образ), работающий контейнер нужно удалить. Это делается командой:
```
docker rm -f octocontainer
```