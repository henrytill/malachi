{
	if ($5 == "A") op = "added"
	else if ($5 == "M") op = "changed"
	else if ($5 == "D") op = "removed"
	else next

	hash = ($5 == "D") ? $3 : $4
	printf "op=%s path=\"%s\" hash=%s\n", op, $6, hash
}
