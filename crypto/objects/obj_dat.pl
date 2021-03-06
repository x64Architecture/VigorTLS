#!/usr/local/bin/perl

# fixes bug in floating point emulation on sparc64 when
# this script produces off-by-one output on sparc64
use integer;

sub obj_cmp
	{
	local(@a,@b,$_,$r);

	$A=$obj_len{$obj{$nid{$a}}};
	$B=$obj_len{$obj{$nid{$b}}};

	$r=($A-$B);
	return($r) if $r != 0;

	$A=$obj_der{$obj{$nid{$a}}};
	$B=$obj_der{$obj{$nid{$b}}};

	return($A cmp $B);
	}

sub expand_obj
	{
	local(*v)=@_;
	local($k,$d);
	local($i);

	do	{
		$i=0;
		foreach $k (keys %v)
			{
			if (($v{$k} =~ s/(OBJ_[^,]+),/$v{$1},/))
				{ $i++; }
			}
		} while($i);
	foreach $k (keys %v)
		{
		@a=split(/,/,$v{$k});
		$objn{$k}=$#a+1;
		}
	return(%objn);
	}

open (IN,"$ARGV[0]") || die "Can't open input file $ARGV[0]";
open (OUT,">$ARGV[1]") || die "Can't open output file $ARGV[1]";

while (<IN>)
	{
	next unless /^\#define\s+(\S+)\s+(.*)$/;
	$v=$1;
	$d=$2;
	$d =~ s/^\"//;
	$d =~ s/\"$//;
	if ($v =~ /^SN_(.*)$/)
		{
		if(defined $snames{$d})
			{
			print "WARNING: Duplicate short name \"$d\"\n";
			}
		else 
			{ $snames{$d} = "X"; }
		$sn{$1}=$d;
		}
	elsif ($v =~ /^LN_(.*)$/)
		{
		if(defined $lnames{$d})
			{
			print "WARNING: Duplicate long name \"$d\"\n";
			}
		else 
			{ $lnames{$d} = "X"; }
		$ln{$1}=$d;
		}
	elsif ($v =~ /^NID_(.*)$/)
		{ $nid{$d}=$1; }
	elsif ($v =~ /^OBJ_(.*)$/)
		{
		$obj{$1}=$v;
		$objd{$v}=$d;
		}
	}
close IN;

%ob=&expand_obj(*objd);

@a=sort { $a <=> $b } keys %nid;
$n=$a[$#a]+1;

@lvalues=();
$lvalues=0;

for ($i=0; $i<$n; $i++)
	{
	if (!defined($nid{$i}))
		{
		push(@out,"{NULL,NULL,NID_undef,0,NULL,0},\n");
		}
	else
		{
		$sn=defined($sn{$nid{$i}})?"$sn{$nid{$i}}":"NULL";
		$ln=defined($ln{$nid{$i}})?"$ln{$nid{$i}}":"NULL";

		if ($sn eq "NULL") {
			$sn=$ln;
			$sn{$nid{$i}} = $ln;
		}

		if ($ln eq "NULL") {
			$ln=$sn;
			$ln{$nid{$i}} = $sn;
		}
			
		$out ="{";
		$out.="\"$sn\"";
		$out.=","."\"$ln\"";
		$out.=",NID_$nid{$i},";
		if (defined($obj{$nid{$i}}) && $objd{$obj{$nid{$i}}} =~ /,/)
			{
			$v=$objd{$obj{$nid{$i}}};
			$v =~ s/L//g;
			$v =~ s/,/ /g;
			$r=&der_it($v);
			$z="";
			$length=0;
			foreach (unpack("C*",$r))
				{
				$z.=sprintf("0x%02X,",$_);
				$length++;
				}
			$obj_der{$obj{$nid{$i}}}=$z;
			$obj_len{$obj{$nid{$i}}}=$length;

			push(@lvalues,sprintf("%-45s/* [%3d] %s */\n",
				$z,$lvalues,$obj{$nid{$i}}));
			$out.="$length,&(lvalues[$lvalues]),0";
			$lvalues+=$length;
			}
		else
			{
			$out.="0,NULL,0";
			}
		$out.="},\n";
		push(@out,$out);
		}
	}

@a=grep(defined($sn{$nid{$_}}),0 .. $n);
foreach (sort { $sn{$nid{$a}} cmp $sn{$nid{$b}} } @a)
	{
	push(@sn,sprintf("%2d,\t/* \"$sn{$nid{$_}}\" */\n",$_));
	}

@a=grep(defined($ln{$nid{$_}}),0 .. $n);
foreach (sort { $ln{$nid{$a}} cmp $ln{$nid{$b}} } @a)
	{
	push(@ln,sprintf("%2d,\t/* \"$ln{$nid{$_}}\" */\n",$_));
	}

@a=grep(defined($obj{$nid{$_}}),0 .. $n);
foreach (sort obj_cmp @a)
	{
	$m=$obj{$nid{$_}};
	$v=$objd{$m};
	$v =~ s/L//g;
	$v =~ s/,/ /g;
	push(@ob,sprintf("%2d,\t/* %-32s %s */\n",$_,$m,$v));
	}

print OUT <<'EOF';
/*
 * WARNING: do not edit!
 * Generated by crypto/objects/obj_dat.pl
 *
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
EOF

printf OUT "#define NUM_NID %d\n",$n;
printf OUT "#define NUM_SN %d\n",$#sn+1;
printf OUT "#define NUM_LN %d\n",$#ln+1;
printf OUT "#define NUM_OBJ %d\n\n",$#ob+1;

printf OUT "static const unsigned char lvalues[%d]={\n",$lvalues+1;
print OUT @lvalues;
print OUT "};\n\n";

printf OUT "static const ASN1_OBJECT nid_objs[NUM_NID]={\n";
foreach (@out)
	{
	if (length($_) > 75)
		{
		$out="";
		foreach (split(/,/))
			{
			$t=$out.$_.",";
			if (length($t) > 70)
				{
				print OUT "$out\n";
				$t="\t$_,";
				}
			$out=$t;
			}
		chop $out;
		print OUT "$out";
		}
	else
		{ print OUT $_; }
	}
print  OUT "};\n\n";

printf OUT "static const unsigned int sn_objs[NUM_SN]={\n";
print  OUT @sn;
print  OUT "};\n\n";

printf OUT "static const unsigned int ln_objs[NUM_LN]={\n";
print  OUT @ln;
print  OUT "};\n\n";

printf OUT "static const unsigned int obj_objs[NUM_OBJ]={\n";
print  OUT @ob;
print  OUT "};\n\n";

close OUT;

sub der_it
	{
	local($v)=@_;
	local(@a,$i,$ret,@r);

	@a=split(/\s+/,$v);
	$ret.=pack("C*",$a[0]*40+$a[1]);
	shift @a;
	shift @a;
	foreach (@a)
		{
		@r=();
		$t=0;
		while ($_ >= 128)
			{
			$x=$_%128;
			$_/=128;
			push(@r,((($t++)?0x80:0)|$x));
			}
		push(@r,((($t++)?0x80:0)|$_));
		$ret.=pack("C*",reverse(@r));
		}
	return($ret);
	}
