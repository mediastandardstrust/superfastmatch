Ext.define('Superfastmatch.store.Fragments', {
    extend: 'Ext.data.Store',
    requires: ['Superfastmatch.model.Fragment'],
    fields: ['from','length','text',{name:'count',defaultValue:1}],
    
    loadDocuments: function(docs){
        var me=this,
            tempStore=Ext.create('Ext.data.Store',{
                model: 'Superfastmatch.model.Fragment',
                getGroupString: function(instance){
                    return instance.get('hash')+":"+instance.get('length');
                }
            });
        me.suspendEvents();
        me.removeAll();
        Ext.each(docs,function(doc){
            doc.fragments().each(function(fragment){
                tempStore.add(fragment); 
            });
        });
        Ext.each(tempStore.getGroups(),function(group){
            var first=group.children[0];
            me.add({
                from: first.get('from'),
                length: first.get('length'),
                text: Ext.String.ellipsis(first.getText(),1000,true),
                count: group.children.length
            })
        });
        me.sort();
        if (me.count()>500){
            var limit=me.getAt(500).get('length');
            me.filterBy(function(record,id){
               return record.get('length')<limit;
            });
        }
        // me.sync();
        me.resumeEvents();
    }
});