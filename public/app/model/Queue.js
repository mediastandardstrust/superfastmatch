Ext.define('Superfastmatch.model.Queue', {
    extend: 'Ext.data.Model',
    requires: ['Ext.ux.data.PagedProxy','Ext.ux.data.TimedJsonReader'],
    uses: ['Superfastmatch.model.Queue'],
    fields: ['id','status','action','priority','doctype','docid','source','target'],

    proxy: {
        type: 'paged',
        startParam: 'start',
        url: '/queue/',
        reader: {
            type: 'timedjson',
            root: 'rows'
        }
    }
});
