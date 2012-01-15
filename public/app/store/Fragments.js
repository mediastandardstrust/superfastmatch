Ext.define('ProcessedFragment', {
    extend: 'Ext.data.Model',
    fields: [
                'from',
                'length',
                'text',
                {name: 'docCount',convert: function(v,record){return Ext.Object.getSize(record.get('documents'))}},
                {name:'count',defaultValue:1},
                'documents'
                ]
});

Ext.define('Superfastmatch.store.Fragments', {
    extend: 'Ext.data.Store',
    requires: ['Superfastmatch.model.Fragment','Ext.ux.data.PagingMemoryProxy'],
    model: 'ProcessedFragment',
    remoteSort: true,
    remoteFilter: true,
    proxy: {
        type: 'pagingmemory'
    },
    
    loadDocuments: function(docs){
        console.log("started");
        var me=this,
            records=[],
            tempStore=Ext.create('Ext.data.Store',{
                model: 'Superfastmatch.model.Fragment',
                getGroupString: function(instance){
                    return instance.get('hash')+":"+instance.get('length');
                }
            });
        Ext.each(docs,function(doc){
            doc.fragments().each(function(fragment){
                tempStore.add(fragment); 
            });
        });
        Ext.each(tempStore.getGroups(),function(group){
            var first=group.children[0],
                documents={};
            Ext.each(group.children,function(child){
                var document=child.getDocument();
                documents[document.get('doctype')+':'+document.get('docid')]=1;
            });
            records.push({
                from: first.get('from'),
                length: first.get('length'),
                text: Ext.String.ellipsis(first.getText(),100,true),
                count: group.children.length,
                documents: documents
            });
        });
        me.getProxy().data=records;
        console.log("finished");
    }
});