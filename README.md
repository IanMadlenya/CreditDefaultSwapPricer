# Credit Default Swap Pricer
Credit Default Swap Pricer project brings together the [ISDA CDS pricier](http://www.cdsmodel.com/cdsmodel/) and some new IMM date modules that are needed to make quick use of the underlying C library functions. This wrapper is aimed at analysts whom want to get up and running very quickly to price and compute risk on CDS using either Python or C++ calling code. The measures computed support a range of potential analysis including:

 + PVDirty, PVClean & Accrued Interest to support NAV calculations & back tests.
 + CS01 & DV01 sensitivities for risk exposure & limit monitoring analysis.
 + Roll sensitivities over range of dates.
 + PVBP sensitivities to support credit risk hedging analysis.

Potential future measures might include Equivalent Notional, Par Spread and Risky CS01, these measures are likely to be added as part of the next full release candidate.


## UnitTest framework

A new TestCdsPricer class has been added to the project which aims to lock in the behaviour of the CDS model relative to the approved MarkIT partners calculator. Since the rules of how to wire together the internal ISDA model api calls can introduce potential error; being able to confirm that the exact behaviour converges with the approved model is essential. 

The test cases are setup to asset values that have been hand validated to 11 decimal places with the MarkIT and BBG calculators. The signs have also been validated against market convension.

```
self.assertAlmostEquals(-1.23099324435, pv_dirty)
self.assertAlmostEquals(-1.19210435546, pv_clean)
self.assertAlmostEquals(0.0388888888889, ai)
self.assertAlmostEquals(14014.5916905, cs01 * 1.0e6)
self.assertAlmostEquals(131.61798715, dv01 * 1.0e6)


```

### Unit test results

The output above details five core tests output from the pricer; these are the net present value with accrued (pv_dirty), the associated clean npv and accrued. The sum of which, clean plus accrued should match the dirty pv or price. Finally two sensitivities are validated, credit spread and interest rate movements.

## Getting Started on Windows

This repo includes a make.bat file intended to build the project on most Windows platforms. The make.bat file requires that you first download and install the first two items in the list below. You might already have Python27 installed, the make.bat file assumes this is installed in the normal C:\Python27\ location as well as a POSIX compliant compiler, MinGW. Microsoft Visual C++ compile cl.exe was tested but comes with a large number of language differences and windows specific issues. MinGW offers a cleaner migration path from Linux to the Windows platform.

+ [Python Installer](https://www.python.org/downloads/release/python-2713/)
+ [MinGW Installer](https://sourceforge.net/projects/mingw/files/Installer/mingw-get-setup.exe/download?use_mirror=kent&r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fmingw%2Ffiles%2FInstaller%2F&use_mirror=kent)
+ [Swig Installer](https://sourceforge.net/projects/swig/?source=typ_redirect)

### Steps to Test 

Use the following steps to clone, make and then finally test the project.

```
git clone https://github.com/bakera1/CreditDefaultSwapPricer.git
cd CreditDefaultSwapPricer
make.bat
```

The build step should result in the _isda.pyd and isda.py file being copied into the cds directory. You can then test the binary with the following steps

```
python isda_test.py
```

The following example output should then be shown on the console from the Python script.

```
C:\github\CreditDefaultSwapPricer\cds>python isda_test.py
23/01/2018      pv_dirty (-1.07226)     cs01 (-8298.48) dv01 (-9.87001e-05)     pvbp5y 0.00049088    5yeqnot (16.9053)  1day roll (-1806.69)    time (32.0)
24/01/2018      pv_dirty (-1.0707)      cs01 (-8286.24) dv01 (-9.78226e-05)     pvbp5y 0.000490599   5yeqnot (16.8901)  1day roll (-1773.74)    time (51.0)
25/01/2018      pv_dirty (-1.06917)     cs01 (-8274.28) dv01 (-9.78311e-05)     pvbp5y 0.000490335   5yeqnot (16.8747)  1day roll (-5329.83)    time (60.0)
26/01/2018      pv_dirty (-1.06457)     cs01 (-8238.78) dv01 (-9.78402e-05)     pvbp5y 0.000489523   5yeqnot (16.8302)  1day roll (-1798.08)    time (66.0)
29/01/2018      pv_dirty (-1.06302)     cs01 (-8226.13) dv01 (-9.69741e-05)     pvbp5y 0.000489267   5yeqnot (16.8132)  1day roll (-1784.68)    time (64.0)
30/01/2018      pv_dirty (-1.06148)     cs01 (-8214.07) dv01 (-9.66886e-05)     pvbp5y 0.000488996   5yeqnot (16.7978)  1day roll (-1806.4)     time (50.0)
31/01/2018      pv_dirty (-1.05992)     cs01 (-8201.83) dv01 (-9.58224e-05)     pvbp5y 0.000488714   5yeqnot (16.7825)  1day roll (-1773.78)    time (37.0)
01/02/2018      pv_dirty (-1.0584)      cs01 (-8189.87) dv01 (-9.58308e-05)     pvbp5y 0.000488455   5yeqnot (16.7669)  1day roll (-5329.87)    time (36.0)
02/02/2018      pv_dirty (-1.05379)     cs01 (-8154.38) dv01 (-9.58399e-05)     pvbp5y 0.000487642   5yeqnot (16.7221)  1day roll (-1797.88)    time (32.0)
05/02/2018      pv_dirty (-1.05225)     cs01 (-8141.73) dv01 (-9.4985e-05)      pvbp5y 0.000487387   5yeqnot (16.7049)  1day roll (-1784.61)    time (35.0)
06/02/2018      pv_dirty (-1.05071)     cs01 (-8129.68) dv01 (-9.47032e-05)     pvbp5y 0.000487119   5yeqnot (16.6893)  1day roll (-1806.11)    time (33.0)
07/02/2018      pv_dirty (-1.04915)     cs01 (-8117.44) dv01 (-9.38483e-05)     pvbp5y 0.000486838   5yeqnot (16.6738)  1day roll (-1773.82)    time (31.0)
08/02/2018      pv_dirty (-1.04762)     cs01 (-8105.49) dv01 (-9.38566e-05)     pvbp5y 0.000486574   5yeqnot (16.6583)  1day roll (-5329.91)    time (41.0)
09/02/2018      pv_dirty (-1.04302)     cs01 (-8069.99) dv01 (-9.38657e-05)     pvbp5y 0.000485761   5yeqnot (16.6131)  1day roll (-1797.68)    time (36.0)
12/02/2018      pv_dirty (-1.04148)     cs01 (-8057.36) dv01 (-9.3022e-05)      pvbp5y 0.000485506   5yeqnot (16.5958)  1day roll (-1784.55)    time (33.0)
13/02/2018      pv_dirty (-1.03994)     cs01 (-8045.31) dv01 (-9.27439e-05)     pvbp5y 0.000485238   5yeqnot (16.5801)  1day roll (-1805.82)    time (36.0)

C:\github\CreditDefaultSwapPricer\cds>history

```

### Possible bug in Python pyconfig.h

You might see the following error message when executing the make.bat file on windows, if this is the case then I suggest that you make a small modification to work around this problem.

[c:\mingw\lib\gcc\mingw32\6.3.0\include\c++\cmath:1157:11: error: '::hypot' has not been declared](https://stackoverflow.com/questions/42276984/hypot-has-not-been-declared)

Edit the file C:\Python27\include\pyconfig.h to comment out line #286 as below; this allows the compilation and linking to complete.

```
#if (__GNUC__==2) && (__GNUC_MINOR__<=91)
#warning "Please use an up-to-date version of gcc! (>2.91 recommended)"
#endif

#define COMPILER "[gcc]"
/*#define hypot _hypot*/
#define PY_LONG_LONG long long
```

## Why create another CDS Pricing library?

The idea behind this library is ease of use, the underlying [ISDA C functions](http://www.cdsmodel.com/cdsmodel/) whilst usable are pretty difficult to integrate and often folks revert to either [3rd party](https://www.google.co.uk/search?q=fincad+cds+pricier&oq=fincad+cds+pricier&aqs=chrome..69i57j0.3457j0j7&sourceid=chrome&ie=UTF-8) or [open source CDS pricing libraries](http://quantlib.org/index.shtml). Whilst this is fine for most uses; when you need precision pricing quickly and easily that conforms exactly to the ISDA CDS model then this wrapper allows you to very quickly build and start writing code Python and price and compute risk on real CDS positions.

1. Is this not just another CDS pricier?

   This library is really only a thin wrapper around the underlying [ISDA CDS Pricing library](http://www.cdsmodel.com/cdsmodel/). The complexity of wiring the spread, interest rate and pricing routines together with some array passing and imm date logic completes an integration task. None of these steps is particularly difficult but together they build a barrier to adoption of the ISDA CDS pricier. By making this library available to use along side the existing [ISDA CDS pricier](http://www.cdsmodel.com/cdsmodel/) it is hoped to lower the barrier and make adoption much easier.

2. Is the only system that can model the weather really only the weather?

   If what you need is safe accurate ISDA pricing then why settle for anything other than the ISDA pricier? however using this CDS pricier avoids the hastle of figuring out all the correct C functions to call and how to pass objects easily into these extern "C" style functions with double* and variety of custom typedef objects like TDateInterval. I just want to create a datetime and pass this into a function right!

## How do I get started? 

The module can be downloaded along with a suitable version of the [ISDA CDS Pricing library](http://www.cdsmodel.com/cdsmodel/) using the make.sh script to invoke the [SWIG](http://www.swig.org/) and gcc builds needed to generate and compile the wrapper and underlying code modules. The g++ invoke is also managed by this file which in turn builds the C++ wrapper ahead of linking the entire module into a library called isda. This libray can then be easily imported into the Python C runtime as shown below.

```python
from isda import cds_all_in_one
```

### CDS All In One

Once you have downloaded and built the project a simple function cds_all_in_one will provide a [SWIG](http://www.swig.org/) wrapped C++ function that invokes the underlying C library functions from the [ISDA CDS model](http://www.cdsmodel.com/cdsmodel/). The interface has been constructed to make the usage as simple and easy as possible. Python native types are used and no custom objects are used. 


#### Array of Arrays

The return response of the cds_all_in_one call has been simplified to return a vector of vectors of doubles, effectively a list of jagged arrays or a jagged matrix. The primary reason being that the python code is much easier to manipulate as a list of tuple objects rather than just a tuple.

+ base - list of primary pricing and risk measures
+ pbvp - forward looking price array
+ roll - array of roll down delta pv values 
+ bucket - array of bucketed cs01 values

```python

from isda import cds_all_in_one_exclude_ir_tenor_dates
from imm import imm_date_vector

# EUR interest rate curve
swap_rates = [-0.00369, -0.00341, -0.00328, -0.00274, -0.00223, -0.00186, 
            -0.00128, 0.00046, 0.00217, 0.003, 0.00504,
            0.00626, 0.00739, 0.00844, 0.00941, 0.01105, 
            0.01281, 0.01436, 0.01506]
swap_tenors = ['1M', '2M', '3M', '6M', '9M', '1Y', '2Y', '3Y', 
            '4Y', '5Y', '6Y', '7Y', '8Y', '9Y, 
            '10Y', '15Y', '20Y', '30Y']

# credit spread curve
credit_spreads = [0.00141154155739384] * 8
credit_spread_tenors = ['6M', '1Y', '2Y', '3Y', '4Y', '5Y', '7Y', '10Y']

# specify n roll tenor
spread_roll_tenors = ['1D', '1W', '1M', '2M', '3M', '4M', '6M', '1Y', '2Y', '3Y', '5Y']
scenario_shifts = [-50, -10, 20, 50, 100, 200, 300]

# value asofdate
sdate = datetime.datetime(2018, 1, 23)
value_date = sdate.strftime('%d/%m/%Y')

# economics of trade
recovery_rate = 0.4
coupon = 100
trade_date = '14/12/2017'
effective_date = '15/12/2017'
maturity_date = '20/12/2011'
accrual_start_date = '20/12/2017'
notional = 1.0 # 1MM EUR
is_buy_protection = 0

# holiday calender for EUR zone
holiday = [datetime.datetime(2017, 12, 25).strftime('%d/%m/%Y'), 
    datetime.datetime(2017, 12, 24).strftime('%d/%m/%Y')]

# numeric tenor list for imm_date_helper
tenor_list = [0.5, 1, 2, 3, 4, 5, 7, 10]

# build imm_dates
imm_dates = [f[1] for f in imm_date_helper(start_date=sdate,
                                           tenor_list=tenor_list)]

# call to cds_all_in_one
base, pvbp, roll, bucket = cds_all_in_one_exclude_ir_tenor_dates(trade_date,
                           effective_date,
                           maturity_date,
                           value_date,
                           accrual_start_date,
                           recovery_rate,
                           coupon,
                           notional,
                           is_buy_protection,
                           swap_rates,
                           swap_tenors,
                           credit_spreads,
                           credit_spread_tenors,
                           spread_roll_tenors,
                           imm_dates,
						   scenario_shifts,
                           holiday,
                           verbose)

# expand return arrays base, pvbp, roll & bucket into discrete variables
pv_dirty, pv_clean, ai, cs01, dv01, duration_in_milliseconds = base
pvbp6m, pvbp1y, pvbp2y, pvbp3y, pvbp4y, pvbp5y, pvbp7y, pvbp10y = pvbp
roll1d, roll1w, roll1m, roll2m, roll3m, roll4m, roll6m, roll1y, roll2y, roll3y, roll5y = roll
bucket_cs01_6m, bucket_cs01_1y, bucket_cs01_2y, bucket_cs01_3y, bucket_cs01_4y, bucket_cs01_5y, bucket_cs01_7y, bucket_cs01_10y = bucket

```

#### Pricing & Risk Measures ####

The cds_all_in_one function call returns a tuple of measures in a positional format, these are detailed as below.

+ pv_dirty - net present value of the CDS, including accrued interest from the current coupon period.
+ pv_clean - net present value of the CDS excluding accrued interest from the current coupon period.
+ ai - accrued interest on the CDS trade in the current coupon period. 
+ cs01 - change in net present value of the CDS, based on a parallel shift of 1bps across the whole CDS spread curve.
+ dv01 - change in net present value of the CDS, based on a parallel shift of 1bps across the whole Interest Rate curve.
+ pvbp6m - present value of a basis point based on a 1bps shift of 6M IMM tenor date.
+ pvbp1y - present value of a basis point based on a 1bps shift of 1Y IMM tenor date.
+ pvbp2y - present value of a basis point based on a 1bps shift of 2Y IMM tenor date.
+ pvbp3y - present value of a basis point based on a 1bps shift of 3y IMM tenor date.
+ pvbp4y - present value of a basis point based on a 1bps shift of 4Y IMM tenor date.
+ pvbp5y - present value of a basis point based on a 1bps shift of 5Y IMM tenor date.
+ pvbp7y - present value of a basis point based on a 1bps shift of 7Y IMM tenor date.
+ pvbp10y - present value of a basis point based on a 1bps shift of 10Y IMM tenor date.
+ duration_in_milliseconds - total wall time in terms of execution of the routine
+ roll1d - 1 day roll down delta PV in base currency of position. 
+ roll1w - 1 week roll down delta PV in base currency of position.
+ roll1m - 1 month roll down delta PV in base currency of position.
+ roll2m - 2 months roll down delta PV in base currency of position.
+ roll3m - 3 months roll down delta PV in base currency of position.
+ roll4m - 4 months roll down delta PV in base currency of position.
+ roll6m - 6 months roll down delta PV in base currency of position.
+ roll1y - 1 year roll down delta PV in base currency of position.
+ roll2y - 2 year roll down delta PV in base currency of position.
+ roll3y - 3 year roll down delta PV in base currency of position.
+ roll5y - 5 year roll down delta PV in base currency of position.
+ bucket_cs01_6m - delta PV of CDS when we move 6m spread tenor by 1bps.
+ bucket_cs01_1y - delta PV of CDS when we move 1y spread tenor by 1bps.
+ bucket_cs01_2y - delta PV of CDS when we move 2y spread tenor by 1bps.
+ bucket_cs01_3y - delta PV of CDS when we move 3y spread tenor by 1bps.
+ bucket_cs01_4y - delta PV of CDS when we move 4y spread tenor by 1bps.
+ bucket_cs01_5y - delta PV of CDS when we move 5y spread tenor by 1bps.
+ bucket_cs01_7y - delta PV of CDS when we move 7y spread tenor by 1bps.
+ bucket_cs01_10y - delta PV of CDS when we move 10y spread tenor by 1bps.

### IMM CDS Dates

Quite often the first hurdle when computing anything related to CDS contracts is how to compute and make available accurate [IMM dates](https://en.wikipedia.org/wiki/IMM_dates) that play nicely with all CDS contracts and business date rules? For this reason this module ships with an imm_date_helper class that takes all the effort away. 

1. imm_date_helper 

   The imm_date_helper function has been written and tested for the explicit purpose of providing accurate IMM dates that comply fully with the CDS market convention. Using the imm_date_helper function you can easily bootstrap the necessary IMM date vector for any business date. 
   
2. [semi-annual roll](https://www.isda.org//2015/12/10/updated-faq-amend-single-name-on-the-run-frequency) 
 
   Since 2015 IMM date logic for CDS contracts has changed to a [semi-annual roll](https://www.isda.org//2015/12/10/updated-faq-amend-single-name-on-the-run-frequency); this change impacts all future tenors along the CDS curve and should be accurately applied to ensure consistent CDS contract pricing.
   
#### Example Semi-Annual IMM Date Roll

The example below shows how the IMM date roll logic is embedded accurately into the helper based on the semi annual roll, with a before and after roll date vector generated along the entire swap curve tenors. If you are pricing and need IMM dates before the ISDA 2015 semi annual roll change then this is automatically applied in the helper function. The function looks at the value of start_date parameter to determine if this latest rule needs to be applied.


```python
    def test_single_roll date_day_before_roll date(self):

        # accepted results
        real_result = [('6M', '20/06/2017'),
                   ('1Y', '20/12/2017'),
                   ('2Y', '20/12/2018'),
                   ('3Y', '20/12/2019'),
                   ('5Y', '20/12/2021'),
                   ('7Y', '20/12/2023')]

        sdate = datetime.datetime(2017, 3, 17)
        tenor_list = [0.5, 1, 2, 3, 5, 7]
        local_result = imm_date_helper(start_date=sdate,
                                 tenor_list=tenor_list,
                                 format='%d/%m/%Y')


        for (r,l) in zip(real_result, local_result):
            self.assertTrue(r[0] == l[0] and r[1] == l[1])

    def test_single_roll date_day_after_roll date(self):

        # accepted results
        real_result = [('6M', '20/12/2017'),
                   ('1Y', '20/06/2018'),
                   ('2Y', '20/06/2019'),
                   ('3Y', '20/06/2020'),
                   ('5Y', '20/06/2022'),
                   ('7Y', '20/06/2024')]

        sdate = datetime.datetime(2017, 3, 20)
        tenor_list = [0.5, 1, 2, 3, 5, 7]
        local_result = imm_date_helper(start_date=sdate,
                                 tenor_list=tenor_list,
                                 format='%d/%m/%Y')

        for (r,l) in zip(real_result, local_result):
            self.assertTrue(r[0] == l[0] and r[1] == l[1])
            
```


