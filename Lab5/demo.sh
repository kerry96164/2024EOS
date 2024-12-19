#!/bin/bash

SESSION="session"
PORT=4444

make clean
make

tmux has-session -t $SESSION 2>/dev/null

if [[ $? -eq 0 ]]
then
	tmux kill-session -t $SESSION
fi

tmux new-session -d -s $SESSION

tmux split-window -v -p 75
tmux split-window -v -p 66
tmux split-window -v -p 50

tmux send-keys -t 0 "./lab5 $PORT" C-m
sleep 1

for i in {1..3}
do
	tmux send-keys -t $i "nc localhost $PORT" C-m
done

tmux -2 attach-session -t $SESSION

make clean

# 檢查是否有 zombie process
sleep 2  # 等待所有程序結束
echo "檢查 zombie process:"
ps aux | grep defunct | grep -v grep

if [[ $? -eq 0 ]]
then
    echo "發現 zombie process。"
else
    echo "未發現 zombie process。"
fi