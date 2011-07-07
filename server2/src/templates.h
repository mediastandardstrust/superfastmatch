#ifndef _SFMTEMPLATES_H                       // duplication check
#define _SFMTEMPLATES

#include <ctemplate/template.h>

using namespace ctemplate;

namespace superfastmatch{
	// Register standard includes
	RegisterTemplateFilename(HEADER, "header.tpl");
	RegisterTemplateFilename(FOOTER, "footer.tpl");

	// Register pages
	RegisterTemplateFilename(SEARCH_PAGE, "search_page.tpl");
	RegisterTemplateFilename(STATUS_PAGE, "status_page.tpl");
	RegisterTemplateFilename(INDEX_PAGE, "index_page.tpl");
	RegisterTemplateFilename(DOCUMENTS_PAGE, "documents_page.tpl");
	RegisterTemplateFilename(DOCUMENT_PAGE, "document_page.tpl");
	RegisterTemplateFilename(HELP_PAGE, "help_page.tpl");

	// Register parts
	RegisterTemplateFilename(POSTING_STATS, "posting_stats.tpl");
	RegisterTemplateFilename(PAGING, "paging.tpl");

}

#endif

