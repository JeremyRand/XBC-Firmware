#!/usr/bin/perl

for($i=0; $i<256; $i++) {
    calc($i);
}

sub calc 
{
    my ($val)=@_;

    # $val is percent of "bright" pixels
    # 0 = 0%
    # 255 = 100%
    # Valid range is actually 1-254

    $AEW= int(($val*0xCA + 0xCA/2) / 255);
    if ($AEW < 1) { $AEW= 1; }
    if ($AEW > 0xC9) { $AEW= 0xC9; }
    $AEB = 0xCA - $AEW;

    $percent_white= $AEW/($AEW+$AEB);
    $inverse= inverse($AEW, $AEB);

    # This is unchecked:
    # AE Ratio is AEW (Reg 0x24)/AEB (Reg 0x25), constraints are: 
    #  -  AReg 0x24 + Reg 0x25 >= 0xCA
    #  -  Range is 0x01 - 0xCA
    # Target 0x24 + 0x25 = 0xCA and map val as follows:
    #    0   => 0x01/0xC9
    #    255 => 0xC9/0x01

    printf("val=%d, AEW=0x%x, AEB=0x%x, sum=0x%x, % white %f, inverse %d, diff=%d\n", 
	   $val, $AEW, $AEB, $AEW+$AEB, $percent_white*100, $inverse, $val-$inverse);
}

sub inverse 
{
    my ($aew, $aeb)=@_;
    if ($aew == 0) { $aew= 1; }
    if ($aeb == 0) { $aeb= 1; }
    my $ret= ($aew*255+255/2)/($aeb+$aew);
    return $ret;
}
