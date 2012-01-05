Ext.define('Superfastmatch.controller.Documents', {
    extend: 'Ext.app.Controller',
    refs: [
        {
            ref: 'results',
            selector: '#DocumentPanel #Results'
        },
        {
            ref: 'text',
            selector: '#DocumentPanel #SearchForm'
        }
    ],
    
    init: function() {
        this.control({
              '#DocumentPanel #DocumentBrowser' : {
                  documentselected: this.onDocumentSelected,
                  documentsloading: this.onDocumentsLoading
              },
              '#DocumentPanel > #Results' : {
                  highlightchange: this.onHighlightChange
              }
        });
    },
    
    onDocumentSelected: function(results){
        this.getResults().loadMatches(results);
        this.getText().lockText(results.get('text'));
        this.getText().setLoading(false);
    },
    
    onDocumentsLoading: function(){
        this.getResults().loading();
        this.getText().setLoading(true);
    },
    
    onHighlightChange: function(eventArgs){
        this.getText().highlightChange(eventArgs);
    }, 
});