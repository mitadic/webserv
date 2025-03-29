git fetch --all
sleep 1
for branch in $(git branch -r | grep -v '\->'); do  
    git branch --track "${branch#origin/}" "$branch"
done
sleep 1
git pull --all
