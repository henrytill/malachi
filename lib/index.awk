{
	if ($5 == "A") op = "added"
	else if ($5 == "M") op = "changed"
	else if ($5 == "D") op = "removed"
	else next

	hash = ($5 == "D") ? $3 : $4
	printf "%s\x1F%s\x1F%s\x1F%s\x1E", op, repo, $6, hash
}
