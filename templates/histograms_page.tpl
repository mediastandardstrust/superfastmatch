{{>HEADER}}

<script type="text/javascript">
	google.load('visualization', '1', {packages: ['corechart']});
</script>

<h2>Index:</h2>
<ul>
	<li>{{DOC_COUNT}} docs in index</li>
	<li>{{HASH_COUNT}} hashes added to index</li>
	<li>{{AVERAGE_DOC_LENGTH}} average document length</li>	
</ul>

{{>HASH_HISTOGRAM}}

{{>DELTAS_HISTOGRAM}}

{{>FOOTER}}