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
	RegisterTemplateFilename(EMPTY_PAGE, "empty_page.tpl");
	RegisterTemplateFilename(ECHO_PAGE, "echo_page.tpl");
	RegisterTemplateFilename(ERROR_PAGE, "error_page.tpl");
	RegisterTemplateFilename(NOT_FOUND_PAGE, "not_found.tpl");
	RegisterTemplateFilename(SEARCH_PAGE, "search_page.tpl");
	RegisterTemplateFilename(QUEUE_PAGE, "queue_page.tpl");
	RegisterTemplateFilename(QUEUED_PAGE, "queued_page.tpl");
	RegisterTemplateFilename(RESULTS_PAGE, "results_page.tpl");
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

