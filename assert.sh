$1
EXIT_CODE=$?
if [ $EXIT_CODE -eq $3 ]; then
  echo "OK: $EXIT_CODE == $3"
else
  echo "NG: $EXIT_CODE != $3"
  exit 1
fi