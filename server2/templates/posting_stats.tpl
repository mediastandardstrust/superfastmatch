<script type="text/javascript">
	google.load('visualization', '1', {packages: ['corechart']});
</script>

<h2>Index:</h2>
<ul>
	<li>{{DOC_COUNT}} docs in index</li>
	<li>{{HASH_COUNT}} hashes added to index</li>
	<li>{{AVERAGE_DOC_LENGTH}} average document length</li>	
</ul>
{{#HISTOGRAM}}
<script type="text/javascript">
function drawVisualization() {
	var data = new google.visualization.DataTable();
	{{#COLUMNS}}
	data.addColumn('number', 'Doc type {{DOC_TYPE}}');
	{{/COLUMNS}}
	{{#ROWS}}
	data.addRow([
		{{#COLUMN}}
		{{DOC_COUNTS}}
		{{#COLUMN_separator}}, {{/COLUMN_separator}}
		{{/COLUMN}}
	]);
	{{/ROWS}}
	var chart = new google.visualization.ColumnChart(document.getElementById('visualization'));
	chart.draw(data, {width: 1500, height: 400,vAxis: {title: "Count"},hAxis: {title: "Docs per hash	",}});
	google.visualization.events.addListener(chart, 'onmouseover', barMouseOver);
	google.visualization.events.addListener(chart, 'onmouseout', barMouseOut);
	function barMouseOver(e) {
		chart.setSelection([e]);
		document.getElementById("chartInfo").innerHTML=e.row+" hashes have "+data.getValue(e.row,e.column)+" documents";
	}
	function barMouseOut(e) {
		chart.setSelection([{'row': null, 'column': null}]);
		document.getElementById("chartInfo").innerHTML="";
	}
}
google.setOnLoadCallback(drawVisualization);
</script>
<div id="visualization" style="width: 1500px; height: 400px;"></div>
<span id="chartInfo"></span>
{{/HISTOGRAM}}