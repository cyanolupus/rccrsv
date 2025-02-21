$1
EXIT_CODE=$?
if [ $EXIT_CODE -eq $3 ]; then
  if [ -n "$4" ]; then
    echo -e "$1\n\tOK\t$2 = $EXIT_CODE == $3"
  fi
else
  echo -e "$1\n\tNG\t$2 = $EXIT_CODE != $3"
  exit 1
fi