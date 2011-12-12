Ext.define('Superfastmatch.controller.Searches', {
    extend: 'Ext.app.Controller',
    stores: ['Fragments'],
    refs: [
        {
            ref: 'search',
            selector: '#SearchPanel > searchform'
        },
        {
            ref: 'status',
            selector: '#SearchPanel > searchform > statusbar'
        },
        {
            ref: 'results',
            selector: '#SearchPanel > #Results'
        },
        {
            ref: 'documents',
            selector: '#SearchPanel > #Results > #Documents'
        },
        {
            ref: 'fragments',
            selector: '#SearchPanel > #Results > #Fragments'
        }
    ],
    
    init: function() {
        this.control({
            'searchform button[text=Search]' : {
                click: this.onSearchClick
            }
        });
    },
    
    onSearchClick: function() {
        var form = this.getSearch().getForm();
        if (form.isValid()) {
            this.getStatus().showBusy('Searching...');
            this.getSearch().getEl().mask();
            this.getResults().getEl().mask();
            var search=Ext.create('Superfastmatch.model.Search',form.getFieldValues());
            search.save({
                success: this.onSearchSave,
                scope: this
            });
        }
    },
    
    onSearchSave: function(record,operation){
        this.getStatus().setStatus({
            text: 'Search in '+record.get("responseTime"),
            iconCls: ''
        });
        this.getResults().enable();
        this.getSearch().getEl().unmask();
        this.getResults().getEl().unmask();
        this.getDocuments().reconfigure(record.documents(),record.documents().model.getColumns());
        this.getDocuments().doLayout();
    }
});