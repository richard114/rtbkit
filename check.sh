#!/bin/bash

tmux kill-session -t check
tmux new-session -d -s check 'cd /opt/graphite/; sudo PYTHONPATH=`pwd`/storage/whisper  ./bin/run-graphite-devel-server.py --libs=`pwd`/webapp/ /opt/graphite/'
tmux rename-window 'checker'
#tmux new-window -d -t check:1 -n 'mock_exchange_runner' './build/x86_64/bin/mock_exchange_runner'
if [[ "$*" != *--quiet* ]]
then
    tmux attach -t check
fi
