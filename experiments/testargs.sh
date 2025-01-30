echo "arg1: $1"
NOEXT=$(echo $1 | cut -d '.' -f 1)
echo "arg1 cut: $NOEXT"