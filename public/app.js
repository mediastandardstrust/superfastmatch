Ext.Loader.setConfig({
    enabled: true,
    disableCaching: false,
    paths: {
        'Ext': 'extjs/src',
        'Ext.ux': 'app/ux'
    }
});

Ext.application({
    name: 'Superfastmatch',
    models: ['Fragment','Search','Document'],
    controllers: ['Searches','Documents'],
    requires: ['Superfastmatch.view.MainViewPort'],
    launch: function() {
        Ext.QuickTips.init();
        var cmp1 = Ext.create('Superfastmatch.view.MainViewPort', {
            renderTo: Ext.getBody()
        });
        cmp1.show();
    }
});
