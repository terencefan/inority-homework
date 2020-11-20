<?php // wordfreq.php

  // used for bigram/trigram processing, i.e., when ngram > 1
  $skip = [ "the", "of", "to", "and", "in", "said", "for", "that", "was", "on",
            "he", "is", "with", "at", "by", "it", "from", "as", "be", "were",
            "an", "have", "his", "but", "has", "are", "not", "who", "they", "its",
            "had", "will", "would", "about", "been", "this", "their", "new", "or", "which",
            "we", "more", "after", "us", "percent", "up", "one", "people" ];

    $displaystyle = 'inline';
    $ngram = 1;

    $files = ["w1.html","w2.html","w3.html","w4.html","w5.html","w6.html", ];

    foreach ($files as $file) {

    $content = file_get_contents($file);     // dangerous......!

    // remove special characters (e.g. &nbsp;)
    $content = preg_replace( '/&nbsp;/', '', $content );
    $content = preg_replace( '/&quot;/', '', $content );
    $content = preg_replace( '/&amp;/', '', $content );
    $content = preg_replace( '/&lt;/', '', $content );
    $content = preg_replace( '/&gt;/', '', $content );

    // remove <script>..</script>
    $content = preg_replace( '/<script.+?<\/script>/s', ' ', $content );

    // remove (crudely) all HTML tags <...>
    $content = preg_replace( '/<[^<>]+>/', ' ', $content );

    // isolate all "words" by simply grouping every set of a-z'
    $wlist = preg_split( '/[^a-z\']+/', strtolower( $content ), 0, PREG_SPLIT_NO_EMPTY );

    $wordlist = array();

    foreach ( $wlist as $word )
    {
      if (strlen($word) > 1) {
      print($word);
      print("\n");
      }
    }
    }