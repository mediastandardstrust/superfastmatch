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
        this.getResults().loading();
    },
    
    onResults: function(record,operation){
        this.getResults().loadMatches(record);
    }
});