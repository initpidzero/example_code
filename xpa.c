/* Expat - simple example */

/*
 * This example reads XML document from standard input, and outputs tags,
 * attributes, values  for attributes in tabulated format, and skips text,
 * CDATA, comments, etc. For example:
 *
 * INPUT:
 * <?xml version="1.0"?><parent><tag id="1"><ctag id="1.1" /></tag></parent>
 *
 * OUTOPUT:
 * >parent
 *    >tag
 *      id: 1
 *        >ctag
 *          id: 1
 *
 * If XML parsing error occurs, the program exits immediately with non-zero
 * exit status (might be used for checking if xml is well-formatted).
 * Expat is a stream-oriented parser that calls registered callbacks functions
 * as it recognizes parts of the document.
 */

#include <stdio.h>
#include <stdlib.h>
#include <expat.h>

	void
parse(XML_Parser parser, char c, int isFinal)
{
	if (XML_STATUS_OK == XML_Parse(parser, &c, isFinal ^ 1, isFinal))
		return;

	fprintf( stderr, "ERROR: parsing XML failed at line %lu, pos %lu: %s\n",
			(unsigned long)XML_GetCurrentLineNumber(parser),
			(unsigned long)XML_GetCurrentColumnNumber(parser),
			XML_ErrorString(XML_GetErrorCode(parser)) );
	exit(1);
}

	int XMLCALL
unknownEncodingHandler(void *encodingHandlerData __attribute__((__unused__)),
		const XML_Char *name __attribute__((__unused__)),
		XML_Encoding *info __attribute__((__unused__)))
{
	/* If you know how to handle encoding with this 'name', you have to fill in
	 *      * 'info' and return XML_STATUS_OK
	 *           * You might be interested in iconv library
	 *                */
	return XML_STATUS_ERROR;
}

typedef struct {
	FILE* out;
	int depth;
	int tagInd;
	int attrInd;
} PContext;

/* NB! XML_Char for UTF-16 is wchar_t or unsigned short
 *  */

/* callback for start element, e.g. <tag> */
void XMLCALL
startElementCallback( void *context,
		const XML_Char *name,
		const XML_Char **atts )
{
	int is_key = 1;
	PContext *ctxt;

	ctxt = (PContext*)context;
	fprintf(ctxt->out, "%*s%c%s\n", ctxt->depth, "", '>', name);
	while (*atts)
	{
		if (is_key)
		{
			fprintf(ctxt->out, "%*c%s: ",
					ctxt->depth + ctxt->attrInd, ' ', *atts);
		}
		else
		{
			fprintf(ctxt->out, "%s\n", *atts);
		}
		is_key = !is_key;
		++atts;
	}
	ctxt->depth += ctxt->tagInd;
}

/* callback for end elements, e.g. </tag>,
 *  * it is called for empty elements, too
 *   */
void XMLCALL
endElementCallback( void *context,
		const XML_Char *name __attribute__((__unused__)) )
{
	PContext *ctxt = (PContext*)context;
	ctxt->depth -= ctxt->tagInd;
}

int main()
{
	XML_Parser parser;
	const char* encoding = NULL;
	PContext ctxt;
	char c;

	/* Create parser.
	 * The only argument for XML_ParserCreate is encoding, and if it's NULL,
	 * then encoding declared in the document is used. Expat supprots natively
	 * US-ASCII, UTF-8, UTF-16, and ISO-8859-1. If any other encoding is
	 * detected, expat calls UnknownEncodingHandler().
	 */
	parser = XML_ParserCreate(encoding);
	if (!parser)
	{
		fprintf(stderr, "ERROR: can't create expat XML parser\n");
		exit(1);
	}

	/* set callback for unknown encoding */
	XML_SetUnknownEncodingHandler(parser, &unknownEncodingHandler, NULL);

	/* Set context that will be passed by the parsers to all handlers */
	ctxt.out = stdout;
	ctxt.depth = 0;
	ctxt.tagInd = 4;
	ctxt.attrInd = 2;
	XML_SetUserData(parser, &ctxt);

	/* set callback for start element */
	XML_SetStartElementHandler(parser, &startElementCallback);

	/* set callback for start element */
	XML_SetEndElementHandler(parser, &endElementCallback);

	/* If you'd like to read input by large blocks, you can have a look at
	 * XML_GetBuffer and XML_ParseBuffer functions.
	 */
	while( EOF != (c = fgetc(stdin)) )
	{
		parse(parser, c, 0);
	}

	/* Finish parsing, note the last argument or XML_Parse */
	parse(parser, c, 1);

	/* Free resource used by expat */
	XML_ParserFree(parser);

	return 0;
}
/* Linking against expat library required (e.g., -lexpat) */
