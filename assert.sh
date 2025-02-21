$1
EXIT_CODE=$?
if [ $EXIT_CODE -eq $3 ]; then
  echo -e "OK\t$2 = $EXIT_CODE == $3"
else
  echo -e "NG\t$2 = $EXIT_CODE != $3"
  exit 1
fi