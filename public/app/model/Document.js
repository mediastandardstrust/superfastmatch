Ext.define('Superfastmatch.model.Document', {
    extend: 'Ext.data.Model',
    requires: ['Ext.ux.data.PagedProxy','Ext.ux.data.TimedJsonReader'],
    uses: ['Superfastmatch.model.Fragment'],
    
    proxy: {
        type: 'paged',
        url: '/document/',
        buildUrl: function(request){
            if (request.operation.doctypes){
                request.url=this.getUrl(request)+request.operation.doctypes+'/';
            }
            return this.superclass.buildUrl.apply(this,arguments);
        },
        reader: {
            type: 'timedjson',
            root: 'rows'
        }
    },
    associations: [{
            type: 'hasMany',
            model: 'Superfastmatch.model.Fragment',
            name: 'fragments',
            reader: 'array'
        },{
            type: 'belongsTo', 
            model: 'Superfastmatch.model.Search', 
            getterName:"getSearch"
        }
    ],
    statics:{
        getColumns: function(){
            var columns=[{header:'Doc Type',dataIndex:'doctype',minWidth:15},{header:'Doc Id',dataIndex:'docid',minWidth:15},{header:'Characters',dataIndex:'characters',minWidth:20}];
            this.getFields().each(function(field){
                if ((field.name!='doctype')&&(field.name!='docid')&&(field.name!='characters')){
                    columns.push({header:field.name.humanize(),dataIndex:field.name});
                }
            });
            return columns;
        }   
    }
});