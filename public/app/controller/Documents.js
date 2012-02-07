Ext.define('Superfastmatch.controller.Documents', {
    extend: 'Ext.app.Controller',
    refs: [
        {
            ref: 'tabs',
            selector: '#Tabs'
        },
        {
            ref: 'browser',
            selector: '#DocumentPanel #DocumentBrowser'
        },
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
              },
              '#Results' :{
                  showdocument: this.onDocumentShow
              }
        });
    },

    onDocumentShow: function(doc){
      this.getTabs().setActiveTab('DocumentPanel');
      this.getBrowser().load(doc);
    },

    onDocumentSelected: function(results){
        if(results){
            this.getResults().loadMatches(results);
            this.getText().lockText(results.get('text'));
            this.getText().setLoading(false);   
        }else{
            this.getResults().clearMatches();
            this.getText().lockText("No Matching Text");
        }
    },
    
    onDocumentsLoading: function(){
        this.getResults().loading();
        this.getText().setLoading(true);
    },
    
    onHighlightChange: function(eventArgs){
        this.getText().highlightChange(eventArgs);
    }, 
});