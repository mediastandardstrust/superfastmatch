<!DOCTYPE html> 
<html>
	<head>
	    <meta charset='utf-8'>
		<title>Superfastmatch | Help</title>
   		<script src='http://html5slides.googlecode.com/svn/trunk/slides.js'></script>
		<style>
			.slides.template-superfastmatch > article{
			  	background: url(http://mediastandardstrust.org/wp-content/themes/mst/images/logo.png) 900px 550px no-repeat,
			  				url(http://assets.sunlightlabs.com.s3.amazonaws.com/site3.1/images/logo.png) 20px 580px no-repeat !important;
				background-color: white !important;  
			}
			table td,table th{
				font-size:50%;
			}
	  	</style>
	</head>
	<!--
	  Google HTML5 slide template

	  Authors: Luke Mahé (code)
	           Marcin Wichary (code and design)

	           Dominic Mazzoni (browser compatibility)
	           Charles Chen (ChromeVox support)

	  URL: http://code.google.com/p/html5slides/
	-->
	<body style='display: none'>
   	 <section class='slides layout-widescreen template-superfastmatch'> 
	
	      <article> 
	        <h1> 
	          Superfastmatch
			  <br/>
			  A text comparison tool
			</h1>
	        <p> 
	          Donovan Hide
	          <br> 
	          July 2011
	        </p> 
	      </article> 

		  <article>
				<h3>
					Browser Extension Specification
				</h3>
				
				<section>
					<p>A <a href="http://code.google.com/chrome/extensions/overview.html">Chrome Extension</a> that will use the <a href="http://code.google.com/p/arc90labs-readability/">Readability</a> script to extract the substance of a news article for a white-listed set of sites, eg:</p>
					<ul>
						<li><a href="http://www.nytimes.com/">New York Times</a></li>
						<li><a href="http://www.washingtonpost.com">Washington Post</a>
					</ul>
					<p>Use <a href="http://github.com/FesterCluck/murmurhash-js/blob/master/murmurhash2_gc.js">Murmur Hash</a> to create ordered integer representation of news article. Use <a href="http://www.movable-type.co.uk/scripts/sha256.html">SHA-256</a> to create a signature for the hashes. Send the hashes and signature to Superfastmatch server. Do a search and association using the hashes and store only the results against the signature. Return the results to the extension. Future users who see the same content (not necessarily the same URL) will get the results from the cache rather than execute another search.</p>
				</section>
		  </article>
		
		  <article>
				<h3>
					Browser Extension Specification II
				</h3>
				
				<section>
					<p>The results (ie. press releases) will be formatted in the browser extension with statistics, much like a <a href="http://churnalism.com">Churnalism.com</a> <a href="http://churnalism.com/45c86/">result.</a></p>
					<p>A <a href="http://addons.mozilla.org/en-US/developers/">Firefox addon</a> should be straight-forward once the Chrome version is complete</p>
				</section>
		  </article>
		
		  <article>
				<h3>
					Scraper Specification
				</h3>
				
				<section>
					<p>Initially 5 scrapers for major sources of press releases in the US. Written in Python, they will extract the publication date, url and title and content from each document and POST it to the Superfastmatch server. The press releases will be searchable within a matter of seconds.</p>
					<p>Potentially the scrapers can check if content has been removed and DELETE the document from the index. Requires that state be stored somewhere.</p>
					<p>Would recommend at least one Science scraper of a source such as <a href="http://www.eurekalert.org/">Eureka Alert</a> - this is an area that raises a lot of concerns in journalism.</p>
				</section>
		  </article>

		  <article>
				<h3>
					Superfastmatch Specification
				</h3>
				
				<section>
					<p>A C++ daemon that stores documents in memory-efficient form for fast searching and ranking according to degree of duplication between search text and document text. Results are presented in faceted form according to document type, which allows for cross-corpora search.</p>
					<p>Leverages <a href="http://en.wikipedia.org/wiki/Collision_(computer_science)">hash collisions</a> as a means of increasing the compression potential through the side-effect of shorter document id deltas. Uses signal processing concepts to filter out collisions at search time.</p>
					<p>Multi-threaded indexing means that a thread is assigned a slot of the hash table and works on it independently without a shared lock. Allows for scaling in line with number of processor cores.</p>
					<p>Asynchronous queued document submission means scrapers don't have to wait for indexing to occur</p>
				</section>
          </article>

		  <article>
				<h3>
					Superfastmatch Specification II
				</h3>
				
				<section>
					<p>Configurable hash width means the more memory that is available to the server, the less collisions that will occur. This increases the speed of search at the cost of memory usage. Also means that lesser-spec machines can run the index with a smaller hash width.</p>
					<p>Requires experimentation to find correct width for a corpus. Hash width can be changed without reloading the documents.</p>
					<p>Whole index is stored in memory, which gives massive speed boost. Currently memory/corpora size ratio is about 2:1. due to <a href="http://code.google.com/p/leveldb/source/browse/trunk/util/coding.h">Variable Length Integer Coding</a>. Much higher compression is <a href="http://www.ittc.ku.edu/~jsv/Papers/Vit87.jacmACMversion.pdf">available</a> if a frequency distribution of doc id deltas is used to create a Huffman-type tree.</p>
				</section>
          </article>

		  <article>
				<h3>
					Superfastmatch dependencies
				</h3>
				
				<section>
					<ul>
						<li><a href="http://fallabs.com/kyotocabinet/">Kyoto Cabinet</a> for the storage of documents, hashes, metadata, incoming queue and associations and threading framework.</li>
						<li><a href="http://fallabs.com/kyototycoon/">Kyoto Tycoon</a> for the HTTP and network framework.</li>
						<li><a href="http://code.google.com/p/google-sparsehash//">Google sparsetable</a> for the construction of the slotted hash table.</li>
						<li><a href="http://code.google.com/p/google-gflags/">Google gflags</a> for command line configuraion.</li>
						<li><a href="http://code.google.com/p/google-ctemplate/">Google ctemplate</a> for rendering of JSON and HTML templates.</li>
						<li><a href="http://code.google.com/p/google-perftools/">Google perftools</a> for memory allocation and profiling</li>
						<li><a href="http://code.google.com/p/leveldb/">Google leveldb</a> as an alternative to Kyoto Cabinet.</li>
					</ul>
				</section>
          </article>

	      <article class='smaller'> 
	        <h3> 
				REST Specification
	        </h3> 
        
			<table>
				<thead>
					<tr>
						<th>
							URL
						</th>
						<th>
							METHOD
						</th>		
						<th>
							DESCRIPTION
						</th>
						<th>
							RESPONSE
						</th>
					</tr>
				</thead>
				<tbody>
					<tr>
						<td>/</td>
						<td>GET</td>
						<td>Returns home page with search form</td>
						<td>200</td>
					</tr>
					<tr>
						<td>/document/</td>
						<td>GET</td>
						<td>Returns the first page of documents</td>
						<td>200</td>
					</tr>
					<tr>
						<td>/document/?cursor=&lt;cursor&gt;</td>
						<td>GET</td>
						<td>Returns the page of documents beginning with &lt;cursor&gt;</td>
						<td>200</td>
					</tr>
					<tr>
						<td>/document/&lt;doctype&gt;/</td>
						<td>GET</td>
						<td>Returns the first page of documents with &lt;doctype&gt;></td>
						<td>200</td>
					</tr>
					<tr>
						<td>/document/&lt;doctype&gt;/?cursor=&lt;cursor&gt;</td>
						<td>GET</td>
						<td>Returns the page of documents with &lt;doctype&gt; beginning with &lt;cursor&gt;</td>
						<td>200</td>
					<tr>
						<td>/document/&lt;doctype&gt;/&lt;docid&gt;/</td>
						<td>GET</td>
						<td>Returns the document with &lt;doctype&gt; and &lt;docid&gt; if exists</td>
						<td>200/404/304</td>
					</tr>
					<tr>
						<td>/document/&lt;doctype&gt;/&lt;docid&gt;/</td>
						<td>POST</td>
						<td>Create or update the document with &lt;doctype&gt; and &lt;docid&gt; and defer association
						<td>202</td>
					</tr>
					<tr>
						<td>/document/&lt;doctype&gt;/&lt;docid&gt;/</td>
						<td>PUT</td>
						<td>Create or update the document with &lt;doctype&gt; and &lt;docid&gt; and associate</td>
						<td>202</td>
					</tr>
					<tr>
						<td>/document/&lt;doctype&gt;/&lt;docid&gt;/</td>
						<td>DELETE</td>
						<td>Delete the document with &lt;doctype&gt; and &lt;docid&gt; and related associations</td>
						<td>204/404</td>
					</tr>
					<tr>
						<td>/index/</td>
						<td>GET</td>
						<td>Returns the first page of the index</td>
						<td>200</td>
					</tr>
					<tr>
						<td>/index/?cursor=&lt;cursor&gt;</td>
						<td>GET</td>
						<td>Returns the page of the index beginning with &lt;cursor&gt;</td>
						<td>200</td>
					</tr>
				</tbody>
			</table>
		</article>

		<article class='smaller'> 
	    	<h3> 
				REST Specification Part II
	        </h3> 
			<table>
				<thead>
					<tr>
						<th>
							URL
						</th>
						<th>
							METHOD
						</th>		
						<th>
							DESCRIPTION
						</th>
						<th>
							RESPONSE
						</th>
					</tr>
				</thead>
				<tbody>
					<tr>
						<td>/association/</td>
						<td>GET</td>
						<td>Returns the first page of associations</td>
						<td>200</td>
					</tr>
					<tr>
						<td>/association/?cursor=&lt;cursor&gt;</td>
						<td>GET</td>
						<td>Returns the page of associations beginning with &lt;cursor&gt;</td>
						<td>200</td>
					</tr>
					<tr>
						<td>/association/&lt;doctype&gt;/</td>
						<td>GET</td>
						<td>Returns the the first page of associations for &lt;doctype&gt;</td>
						<td>200</td>
					</tr>
					<tr>	
						<td>/association/&lt;doctype&gt;/?cursor=&lt;cursor&gt;</td>
						<td>GET</td>
						<td>Returns the the page of associations for &lt;doctype&gt; beginning with &lt;cursor&gt;</td>
						<td>200</td>
					</tr>
					<tr>
						<td>/association/&lt;doctype&gt;/&lt;docid&gt;/</td>
						<td>GET</td>
						<td>Returns the associations for a specific document</td>
						<td>200/304</td>
					</tr>
					<tr>
						<td>/association/</td>
						<td>POST</td>
						<td>Updates associations for all documents</td>
						<td>202</td>
					</tr>
					<tr>
						<td>/association/&lt;doctype&gt;/</td>
						<td>POST</td>
						<td>Updates associations for all documents with &lt;doctype&gt;</td>
						<td>202</td>
					</tr>
					<tr>
						<td>/association/&lt;doctype1&gt;/&lt;doctype2&gt;/</td>
						<td>POST</td>
						<td>Updates associations for all documents with &lt;doctype1&gt; against &lt;doctype2&gt;</td>
						<td>202</td>
					</tr>
					<tr>
						<td>/search/</td>
						<td>POST</td>
						<td>Returns the results for a search of all documents</td>
						<td>200</td>
					</tr>
					<tr>
						<td>/search/&lt;doctype&gt;/</td>
						<td>POST</td>
						<td>Returns the results for a search against documents of &lt;doctype&gt;</td>
						<td>200</td>
					</tr>
					<tr>
						<td>/status/</td>
						<td>GET</td>
						<td>Returns counts for each DB</td>
						<td>200</td>
					</tr>
				</tbody>
			</table>
		</article>
		<article class='smaller'> 
	    	<h3> 
				REST Specification Part III
	        </h3> 
				<table>
					<thead>
						<tr>
							<th>
								URL
							</th>
							<th>
								METHOD
							</th>		
							<th>
								DESCRIPTION
							</th>
							<th>
								RESPONSE
							</th>
						</tr>
					</thead>
					<tbody>
						<tr>
							<td>/queue/</td>
							<td>GET</td>
							<td>Returns the first page of jobs</td>
							<td>200</td>
						</tr>
						<tr>
							<td>/queue/?cursor=&lt;cursor&gt;</td>
							<td>GET</td>
							<td>Returns the page of jobs beginning with &lt;cursor&gt;</td>
							<td>200</td>
						</tr>
						<tr>
							<td>/queue/&lt;queueid&gt;/</td>
							<td>GET</td>
							<td>Returns the details of &lt;queueid&gt;</td>
							<td>200/404/304</td>
						</tr>
					</tbody>
				</table>
			<p>Where &lt;doctype&gt;, &lt;docid&gt; and &lt;queueid&gt; are integers between 1 and 4294967295 ie. (all unsigned 32 bit numbers, excluding 0)</p>
	      </article>
      
	      <!-- <article> 
	     	        <p> 
	     	          This is a slide with just text. This is a slide with just text.
	     	          This is a slide with just text. This is a slide with just text.
	     	          This is a slide with just text. This is a slide with just text.
	     	        </p> 
	     	        <p> 
	     	          There is more text just underneath.
	     	        </p> 
	     	      </article> 
	      
	     	      <article> 
	     	        <h3> 
	     	          Simple slide with header and text
	     	        </h3> 
	     	        <p> 
	     	          This is a slide with just text. This is a slide with just text.
	     	          This is a slide with just text. This is a slide with just text.
	     	          This is a slide with just text. This is a slide with just text.
	     	        </p> 
	     	        <p> 
	     	          There is more text just underneath with a <code>code sample: 5px</code>.
	     	        </p> 
	     	      </article> 
	      
	     	      <article class='smaller'> 
	     	        <h3> 
	     	          Simple slide with header and text (small font)
	     	        </h3> 
	     	        <p> 
	     	          This is a slide with just text. This is a slide with just text.
	     	          This is a slide with just text. This is a slide with just text.
	     	          This is a slide with just text. This is a slide with just text.
	     	        </p> 
	     	        <p> 
	     	          There is more text just underneath with a <code>code sample: 5px</code>.
	     	        </p> 
	     	      </article> 
	      
	     	      <article> 
	     	        <h3> 
	     	          Slide with bullet points and a longer title, just because we
	     	          can make it longer
	     	        </h3> 
	     	        <ul> 
	     	          <li> 
	     	            Use this template to create your presentation
	     	          </li> 
	     	          <li> 
	     	            Use the provided color palette, box and arrow graphics, and
	     	            chart styles
	     	          </li> 
	     	          <li> 
	     	            Instructions are provided to assist you in using this
	     	            presentation template effectively
	     	          </li> 
	     	          <li> 
	     	            At all times strive to maintain Google's corporate look and feel
	     	          </li> 
	     	        </ul> 
	     	      </article> 
	      
	     	      <article> 
	     	        <h3> 
	     	          Slide with bullet points that builds
	     	        </h3> 
	     	        <ul class="build"> 
	     	          <li> 
	     	            This is an example of a list
	     	          </li> 
	     	          <li> 
	     	            The list items fade in
	     	          </li> 
	     	          <li> 
	     	            Last one!
	     	          </li> 
	     	        </ul> 
	      
	     	        <div class="build"> 
	     	          <p>Any element with child nodes can build.</p> 
	     	          <p>It doesn't have to be a list.</p> 
	     	        </div> 
	     	      </article> 
	      
	     	      <article class='smaller'> 
	     	        <h3> 
	     	          Slide with bullet points (small font)
	     	        </h3> 
	     	        <ul> 
	     	          <li> 
	     	            Use this template to create your presentation
	     	          <li> 
	     	            Use the provided color palette, box and arrow graphics, and
	     	            chart styles
	     	          <li> 
	     	            Instructions are provided to assist you in using this
	     	            presentation template effectively
	     	          <li> 
	     	            At all times strive to maintain Google's corporate look and feel
	     	        </ul> 
	     	      </article> 
	      
	     	      <article> 
	     	        <h3> 
	     	          Slide with a table
	     	        </h3> 
	             
	     	        <table> 
	     	          <tr> 
	     	            <th> 
	     	              Name
	     	            <th> 
	     	              Occupation
	     	          <tr> 
	     	            <td> 
	     	              Luke Mahé
	     	            <td> 
	     	              V.P. of Keepin’ It Real
	     	          <tr> 
	     	            <td> 
	     	              Marcin Wichary
	     	            <td> 
	     	              The Michael Bay of Doodles
	     	        </table> 
	     	      </article> 
	           
	     	      <article class='smaller'> 
	     	        <h3> 
	     	          Slide with a table (smaller text)
	     	        </h3> 
	             
	     	        <table> 
	     	          <tr> 
	     	            <th> 
	     	              Name
	     	            <th> 
	     	              Occupation
	     	          <tr> 
	     	            <td> 
	     	              Luke Mahé
	     	            <td> 
	     	              V.P. of Keepin’ It Real
	     	          <tr> 
	     	            <td> 
	     	              Marcin Wichary
	     	            <td> 
	     	              The Michael Bay of Doodles
	     	        </table> 
	     	      </article> 
	           
	     	      <article> 
	     	        <h3> 
	     	          Styles
	     	        </h3> 
	     	        <ul> 
	     	          <li> 
	     	            <span class='red'>class="red"</span> 
	     	          <li> 
	     	            <span class='blue'>class="blue"</span> 
	     	          <li> 
	     	            <span class='green'>class="green"</span> 
	     	          <li> 
	     	            <span class='yellow'>class="yellow"</span> 
	     	          <li> 
	     	            <span class='black'>class="black"</span> 
	     	          <li> 
	     	            <span class='white'>class="white"</span> 
	     	          <li> 
	     	            <b>bold</b> and <i>italic</i> 
	     	        </ul> 
	     	      </article> 
	           
	     	      <article> 
	     	        <h2> 
	     	          Segue slide
	     	        </h2> 
	     	      </article> 
	      
	     	      <article> 
	     	        <h3> 
	     	          Slide with an image
	     	        </h3> 
	     	        <p> 
	     	          <img style='height: 500px' src='images/example-graph.png'> 
	     	        </p> 
	     	        <div class='source'> 
	     	          Source: Sergey Brin
	     	        </div> 
	     	      </article> 
	      
	     	      <article> 
	     	        <h3> 
	     	          Slide with an image (centered)
	     	        </h3> 
	     	        <p> 
	     	          <img class='centered' style='height: 500px' src='images/example-graph.png'> 
	     	        </p> 
	     	        <div class='source'> 
	     	          Source: Larry Page
	     	        </div> 
	     	      </article> 
	      
	     	      <article class='fill'> 
	     	        <h3> 
	     	          Image filling the slide (with optional header)
	     	        </h3> 
	     	        <p> 
	     	          <img src='images/example-cat.jpg'> 
	     	        </p> 
	     	        <div class='source white'> 
	     	          Source: Eric Schmidt
	     	        </div> 
	     	      </article> 
	      
	     	      <article> 
	     	        <h3> 
	     	          This slide has some code
	     	        </h3> 
	     	        <section> 
	     	        <pre> 
	     	&lt;script type='text/javascript'&gt;
	     	  // Say hello world until the user starts questioning
	     	  // the meaningfulness of their existence.
	     	  function helloWorld(world) {
	     	    for (var i = 42; --i &gt;= 0;) {
	     	      alert('Hello ' + String(world));
	     	    }
	     	  }
	     	&lt;/script&gt;
	     	&lt;style&gt;
	     	  p { color: pink }
	     	  b { color: blue }
	     	  u { color: 'umber' }
	     	&lt;/style&gt;
	     	</pre> 
	     	        </section> 
	     	      </article> 
	           
	     	      <article class='smaller'> 
	     	        <h3> 
	     	          This slide has some code (small font)
	     	        </h3> 
	     	        <section> 
	     	        <pre> 
	     	&lt;script type='text/javascript'&gt;
	     	  // Say hello world until the user starts questioning
	     	  // the meaningfulness of their existence.
	     	  function helloWorld(world) {
	     	    for (var i = 42; --i &gt;= 0;) {
	     	      alert('Hello ' + String(world));
	     	    }
	     	  }
	     	&lt;/script&gt;
	     	&lt;style&gt;
	     	  p { color: pink }
	     	  b { color: blue }
	     	  u { color: 'umber' }
	     	&lt;/style&gt;
	     	</pre> 
	     	        </section> 
	     	      </article> 
	           
	     	      <article> 
	     	        <q> 
	     	          The best way to predict the future is to invent it.
	     	        </q> 
	     	        <div class='author'> 
	     	          Alan Kay
	     	        </div> 
	     	      </article> 
	           
	     	      <article class='smaller'> 
	     	        <q> 
	     	          A distributed system is one in which the failure of a computer 
	     	          you didn’t even know existed can render your own computer unusable.
	     	        </q> 
	     	        <div class='author'> 
	     	          Leslie Lamport
	     	        </div> 
	     	      </article> 
	           
	     	      <article class='nobackground'> 
	     	        <h3> 
	     	          A slide with an embed + title
	     	        </h3> 
	             
	     	        <iframe src='http://www.google.com/doodle4google/history.html'></iframe> 
	     	      </article> 
	      
	     	      <article class='nobackground'> 
	     	        <iframe src='http://www.google.com/doodle4google/history.html'></iframe> 
	     	      </article> 
	      
	     	      <article class='fill'> 
	     	        <h3> 
	     	          Full-slide embed with (optional) slide title on top
	     	        </h3> 
	     	        <iframe src='http://www.google.com/doodle4google/history.html'></iframe> 
	     	      </article> 
	           
	     	      <article> 
	     	        <h3> 
	     	          Thank you!
	     	        </h3> 
	             
	     	        <ul> 
	     	          <li> 
	     	            <a href='http://www.google.com'>google.com</a> 
	     	        </ul> 
	     	      </article>  -->
 
	    </section> 
 	</body>
</html>

