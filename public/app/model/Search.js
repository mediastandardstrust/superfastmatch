Ext.define('Superfastmatch.model.Search', {
    extend: 'Ext.data.Model',
    requires: ['Ext.ux.data.TimedJsonReader','Ext.ux.data.PagedProxy'],
    uses: ['Superfastmatch.model.Document'],

    fields: ['doctype','docid',{name: 'text',persist: 'false'},'responseTime'],
    proxy:{
        type: 'paged',
        url: '/search/',
        // appendId: false,
        reader: {
            type: 'timedjson'
        }
    },
    associations:[{
                    type: 'hasMany',
                    model: 'Superfastmatch.model.Document',
                    name: 'documents',
                    reader:{
                           type: 'json',
                           root: 'rows',
                       }
                }],
            
    // Hack to reload data from save
    // Hopefully extjs 4.1 will solve this
    save: function(options){
        var me=this,
            success=options.success,
            scope=options.scope,
            text=me.get('text');
        options.success=function(record,operation){
            var record=operation.getResultSet().records[0];
            record.set('text',text);
            Ext.callback(success, scope, [record, operation]);
        }
        me.callParent([options]);
    }
});