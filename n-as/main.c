int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		return -1;
	}
	YY_BUFFER_STATE s = yy_scan_string(argv[1]);
	yyparse();
	yy_delete_buffer(s);
	return 0;
}
void yyerror(const char* error)
{
	printf("%s: at char: %d\n",error,char_count);
	exit(-1);
}