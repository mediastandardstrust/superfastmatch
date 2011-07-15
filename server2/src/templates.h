#ifndef _SFMTEMPLATES_H                       // duplication check
#define _SFMTEMPLATES_H

#include <common.h>
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
	RegisterTemplateFilename(HISTOGRAMS_PAGE, "histograms_page.tpl");

	// Register parts
	RegisterTemplateFilename(PAGING, "paging.tpl");
	RegisterTemplateFilename(HISTOGRAM, "histogram.tpl");

}

#endif

