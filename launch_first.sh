#!/bin/bash

tmux kill-session -t redis
tmux new-session -d -s redis '~/local/bin/zookeeper/bin/zkServer.sh start; redis-cli shutdown; redis-server ./rtbkit/sample.redis.conf'
tmux rename-window 'redis zookeeper'
if [[ "$*" != *--quiet* ]]
then
    tmux attach -t redis
fi
