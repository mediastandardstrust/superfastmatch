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
            var columns=[{header:'Doc Type',dataIndex:'doctype',minWidth:60,maxWidth:60},{header:'Doc Id',dataIndex:'docid',minWidth:60,maxWidth:60},{header:'Characters',dataIndex:'characters',minWidth:70,maxWidth:70}];
            Ext.each(this.getFields(),function(field){
                if (!Ext.Array.contains(['doctype','docid','characters','id'],field.name)){
                    if(field.name.search(/date$/)!=-1){
                      columns.push({header:field.name.humanize(),dataIndex:field.name,renderer: function(v){return v?Ext.util.Format.date(new Date(v),'m/d/Y'):'';}});
                    }else if(field.name.search(/url$/)!=-1){
                      columns.push({header:field.name.humanize(),dataIndex:field.name,xtype: 'templatecolumn',tpl: '<a href="{'+field.name+'}" target="_blank">{'+field.name+'}</a>'});
                    }else{
                      columns.push({header:field.name.humanize(),dataIndex:field.name});
                    }
                }
            });
            return columns;
        }   
    }
});