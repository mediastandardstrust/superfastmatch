<script type="text/javascript">
function drawVisualization() {
	var data = new google.visualization.DataTable();
	data.addColumn('string', 'Counts');
	{{#COLUMNS}}
	data.addColumn('number', 'Doc type {{DOC_TYPE}}');
	{{/COLUMNS}}
	{{#ROWS}}
	data.addRow([
		'{{INDEX}}',
		{{#COLUMN}}
		{{DOC_COUNTS}}
		{{#COLUMN_separator}}, {{/COLUMN_separator}}
		{{/COLUMN}}
	]);
	{{/ROWS}}
	var chart = new google.visualization.ColumnChart(document.getElementById('{{NAME}}'));
	chart.draw(data, {width: 1500, height: 400,vAxis: {title: "Count"},hAxis: {title: "{{TITLE}}",}});
	// google.visualization.events.addListener(chart, 'onmouseover', barMouseOver);
	// google.visualization.events.addListener(chart, 'onmouseout', barMouseOut);
	// function barMouseOver(e) {
	// 	chart.setSelection([e]);
	// 	document.getElementById("chartInfo").innerHTML=+data.getValue(e.row,e.column)+" hashes have "+
	// 	e.row+" documents";
	// }
	// function barMouseOut(e) {
	// 	chart.setSelection([{'row': null, 'column': null}]);
	// 	document.getElementById("chartInfo").innerHTML="";
	// }
}
google.setOnLoadCallback(drawVisualization);
</script>
<div id="{{NAME}}" style="width: 1500px; height: 400px;"></div>
<!-- <span id="chartInfo"></span> -->