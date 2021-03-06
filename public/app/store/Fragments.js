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
        // console.log("Started processing documents");
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
                documents={},
                from={},
                to={};
            Ext.each(group.children,function(child){
                var doc=child.getDocument();
                Object.increment(documents,doc.get('doctype')+':'+doc.get('docid'));
                Object.increment(from,doc.get('from'));
                Object.increment(to,doc.get('to'));
            });
            records.push({
                from: first.get('from'),
                length: first.get('length'),
                text: Ext.String.ellipsis(first.getText(),300,true),
                count: group.children.length,
                documents: documents
            });
        });
        me.getProxy().data=records;
        // console.log("Finished processing documents");
    }
});