Ext.define('Superfastmatch.model.Index', {
    extend: 'Ext.data.Model',
    requires: ['Ext.ux.data.PagedProxy','Ext.ux.data.TimedJsonReader'],
    fields: ['hash','bytes'],
    idProperty: 'hash',
    hasMany: {model:'Superfastmatch.model.IndexRow',name:'doctypes'},

    proxy: {
        type: 'paged',
        startParam: 'start',
        url: '/index/',
        reader: {
            type: 'timedjson',
            root: 'rows'
        }
    }
});

Ext.define('Superfastmatch.model.IndexRow',{
    extend: 'Ext.data.Model',
    fields: ['doctype','bytes','docids','deltas'],
    associations: [
        { type: 'belongsTo', model: 'Superfastmatch.model.Index', getterName: 'getIndex' }
    ]
});
