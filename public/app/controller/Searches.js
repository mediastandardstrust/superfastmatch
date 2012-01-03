Ext.define('Superfastmatch.controller.Searches', {
    extend: 'Ext.app.Controller',
    stores: ['Fragments'],
    refs: [
        {
            ref: 'search',
            selector: '#SearchPanel > searchform'
        },
        {
            ref: 'results',
            selector: '#SearchPanel > #Results'
        },
        {
            ref: 'documents',
            selector: '#SearchPanel > #Results > #Documents'
        }
    ],
    
    init: function() {
        this.control({
            '#SearchPanel > searchform' : {
                searching: this.onSearch,
                results:   this.onResults
            },
            '#SearchPanel > #Results' : {
                highlightchange: this.onHighlightChange
            }
        });
    },
    
    onHighlightChange: function(eventArgs){
        this.getSearch().highlightChange(eventArgs);
    },
    
    onSearch: function() {
        this.getResults().getEl().mask();
    },
    
    onResults: function(record,operation){
        this.getResults().enable();
        this.getResults().getEl().unmask();
        this.getDocuments().reconfigure(record.documents(),record.documents().model.getColumns());
        this.getDocuments().doLayout();
    }
});