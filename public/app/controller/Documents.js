Ext.define('Superfastmatch.controller.Documents', {
    extend: 'Ext.app.Controller',
    models: ['Document','Search'],
    stores: ['Documents'],
    refs: [
        {
            ref: 'panel',
            selector: '#DocumentPanel'
        },
        {
            ref: 'browser',
            selector: '#DocumentPanel #DocumentBrowser'
        },
        {
            ref: 'paging',
            selector: '#DocumentPanel #DocumentPaging'
        },
        {
            ref: 'text',
            selector: '#DocumentPanel #Text'
        },
        {
            ref: 'results',
            selector: '#DocumentPanel #Results'
        },
        {
            ref: 'documents',
            selector: '#DocumentPanel #Results > #Documents'
        },
        {
            ref: 'fragments',
            selector: '#DocumentPanel #Results > #Fragments'
        }
    ],
    
    init: function() {
        this.control({
              '#DocumentPanel' : {
                  activate: this.onActivate
              },
              '#DocumentPanel #DocumentBrowser' : {
                  select: this.onSelectDocument
              }
        });
    },

    onActivate: function(){
        var me=this;
        me.getPaging().bindStore(me.getDocumentsStore(),true);
        me.getBrowser().reconfigure(me.getDocumentsStore());
        me.getDocumentsStore().on('load',me.onLoadDocuments,me);
        if (me.getBrowser().getStore().count()==0){
            me.getDocumentsStore().load();
        }
    },
    
    onSelectDocument: function(selModel,selected){
        var me=this,
            id=selected.get('doctype')+'/'+selected.get('docid')+'/';
        me.getSearchModel().load(id,{
            url: '/document/',
            success: me.onLoadResults,
            scope: me
        });
    },
    
    onLoadResults: function(record,operation){
        this.getText().enable();
        this.getText().getEl().unmask();
        this.getText().update({text:record.get('text')});
        this.getResults().enable();
        this.getResults().getEl().unmask();
        this.getDocuments().reconfigure(record.documents(),record.documents().model.getColumns());
        this.getDocuments().doLayout();  
    },
    
    onLoadDocuments: function(store,records,success){
        var me=this;
        me.getBrowser().reconfigure(store,me.getDocumentModel().getColumns());
        if (records.length){
            me.getBrowser().getSelectionModel().select(0);
        }
        me.getPanel().doLayout();
        me.getBrowser().getView().refresh();
    }
});