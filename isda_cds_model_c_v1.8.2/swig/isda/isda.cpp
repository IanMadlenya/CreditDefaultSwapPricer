#include <vector>
#include <iostream>
#include <sstream>
#include "isda.h"
#include "example.h"
#include "busday.h"
#include "dateconv.h"
#include "date_sup.h"
#include <ctime>
#include <stdio.h>
#include "main.h"
#include "tcurve.h"
#include "bastypes.h"
#include <math.h>
#include "macros.h"

using namespace std;

TDate parse_string_ddmmyyyy(const std::string& s, int& day, int& month, int& year)
{
  sscanf(s.c_str(), "%2d/%2d/%4d", &day, &month, &year);
  return JpmcdsDate(year, month, day);
}

TDate parse_string_ddmmyyyy_to_jpmcdsdate(const std::string& s)
{
  int day, month, year;
  sscanf(s.c_str(), "%2d/%2d/%4d", &day, &month, &year);
  return JpmcdsDate(year, month, day);
}
/*
***************************************************************************
** compute basic pricing for a what if cds position.
***************************************************************************
*/

vector< vector<double> > cds_all_in_one (
 string trade_date,					/* (I) trade date of cds as DD/MM/YYYY */
 string effective_date,				/* (I) effective date of cds as DD/MM/YYYY */
 string maturity_date,				/* (I) maturity date of cds as DD/MM/YYYY */
 string value_date,					/* (I) date to value the cds DD/MM/YYYY */
 string accrual_start_date,			/* (I) accrual start date of cds as DD/MM/YYYY */
 double	recovery_rate,				/* (I) recover rate of the curve in basis points */
 double coupon_rate,				/* (I) CouponRate (e.g. 0.05 = 5% = 500bp) */
 double notional,					/* (I) Notional MM */
 int is_buy_protection,				/* (I) direction of credit risk */
 vector<double> swap_rates, 		/* (I) swap rates */
 vector<string> swap_tenors,		/* (I) swap tenors "1M", "2M" */
 vector<string> swap_maturities,	/* (I) swap maturity dates */
 vector<double> spread_rates,		/* (I) spread spreads */
 vector<string> spread_tenors,		/* (I) spread tenors "6M", "1Y" */
 vector<string> spread_roll_tenors, /* (I) spread roll tenors */
 vector<string> imm_dates,			/* (I) imm dates */
 vector<double> scenario_tenors,	/* (I) spread tenors -100, -90, -80, -70 ... */
 int verbose						/* (I) output message text */
)
{

  int start_s = clock();

  // used in risk calculations
  double single_basis_point = 0.0001;

  TDate trade_date_jpm, effective_date_jpm, maturity_date_jpm,
  accrual_start_date_jpm, value_date_jpm;

 // empty curve pointers
  TCurve *zerocurve = NULL;
  TCurve *zerocurve_dv01 = NULL;
  TCurve *spreadcurve = NULL;
  TCurve *spreadcurve_cs01 = NULL;
  TCurve *spreadcurve_dv01 = NULL;

  // discount
  double rates[swap_rates.size()];
  double rates_dv01[swap_rates.size()];
  char *expiries[swap_rates.size()];
  long int maturity[swap_rates.size()];
  long int *pointer_maturity;

  // credit spread
  double spreads[spread_rates.size()];
  double spreads_cs01[spread_rates.size()];
  double *pointer_spreads_cs01;
  double *pointer_spreads;
  long int tenors[imm_dates.size()];
  long int *pointer_tenors;
  double *par_spread_pointer;

  char *spread_roll_expiries[spread_roll_tenors.size()];

  // numeric values
  double coupon_rate_in_basis_points = coupon_rate/10000.0;
  double dirtypv;
  double cleanpv;
  double ai;
  double dirtypv_cs01;
  double dirtypv_dv01;

  // outer return vector
  vector < vector<double> > allinone;
  // inner return vector
  vector <double> allinone_base;
  vector <double> allinone_pvbp;
  vector < vector<double> > allinone_roll;

  // assumes sell protection default
  double credit_risk_direction_scale_factor = 1;

  // jpm roll dates
  TDate *pointer_roll_dates_jpm;
  double roll_pvclean;

  /////////////////////////////
  // parse char* to jpm dates
  /////////////////////////////

  trade_date_jpm = parse_string_ddmmyyyy_to_jpmcdsdate(trade_date);
  effective_date_jpm = parse_string_ddmmyyyy_to_jpmcdsdate(effective_date);
  maturity_date_jpm = parse_string_ddmmyyyy_to_jpmcdsdate(maturity_date);
  accrual_start_date_jpm = parse_string_ddmmyyyy_to_jpmcdsdate(accrual_start_date);
  value_date_jpm = parse_string_ddmmyyyy_to_jpmcdsdate(value_date);

  if (verbose == 1){
	  std::cout << "value_date_jpm " << value_date_jpm << std::endl;
	  std::cout << "trade_date_jpm " << trade_date_jpm << std::endl;
	  std::cout << "effective_date_jpm " << effective_date_jpm << std::endl;
	  std::cout << "accrual_start_date_jpm " << accrual_start_date_jpm << std::endl;
	  std::cout << "maturity_date_jpm " << maturity_date_jpm << std::endl;
  }

  /////////////////////////////
  // bootstrap interest rate curve
  /////////////////////////////

  for(int r = 0; r < swap_rates.size(); r++){
    rates[r] = swap_rates[r];
    rates_dv01[r] = swap_rates[r] + single_basis_point;
  }

  for(int r = 0; r < swap_tenors.size(); r++){
    expiries[r] = (char *)swap_tenors[r].c_str();
  }

  for(int r = 0; r < swap_maturities.size(); r++){
    maturity[r] = parse_string_ddmmyyyy_to_jpmcdsdate(swap_maturities[r].c_str());
    if (verbose){
    	std::cout << r << " swap maturity " << maturity[r] << std::endl;
    }
  }

  pointer_maturity = maturity;

  // bootstrap discount curve
  zerocurve = build_zero_interest_rate_curve2(value_date_jpm
		  , rates
		  , expiries
		  , maturity
		  , verbose);

  zerocurve_dv01 = build_zero_interest_rate_curve2(value_date_jpm
		  , rates_dv01
		  , expiries
		  , maturity
		  , verbose);

  /////////////////////////////
  // bootstrap spread curve
  /////////////////////////////

  // parse imm dates into jpm string format
  for(int r = 0; r < imm_dates.size(); r++){
    tenors[r] = parse_string_ddmmyyyy_to_jpmcdsdate(imm_dates[r]);
    if (verbose){
    	std::cout << r << " imm tenor " << tenors[r] << std::endl;
    }
  }

  // need a pointer to array of long int
  pointer_tenors = tenors;

  for(int r = 0; r < spread_rates.size(); r++){
    spreads[r] = spread_rates[r];
    spreads_cs01[r] = spread_rates[r] + single_basis_point;
    if (verbose){
      std::cout << r << " " << spreads_cs01[r] << std::endl;
    }
  }

  // similarly need a double *
  pointer_spreads = spreads;
  pointer_spreads_cs01 = spreads_cs01;

  // build spread curve
  spreadcurve = build_credit_spread_par_curve(
  			value_date_jpm
  			, zerocurve
  			, effective_date_jpm
  			, pointer_spreads
  			, pointer_tenors
  			, recovery_rate
  			, imm_dates.size()
  			, verbose);

  if (spreadcurve == NULL){
  	std::cout << spreadcurve << std::endl;
  	std::cout << "bad spreadcurve" << std::endl;
  }


  if (spreadcurve != NULL){
    if (verbose){
  	  std::cout << spreadcurve << std::endl;
  	  std::cout << "spreadcurve OK!" << std::endl;
  	}
  }

  // cs01_spread_curve

  // build spread curve
  spreadcurve_cs01 = build_credit_spread_par_curve(
  			value_date_jpm
  			, zerocurve
  			, effective_date_jpm
  			, pointer_spreads_cs01
  			, pointer_tenors
  			, recovery_rate
  			, imm_dates.size()
  			, verbose);

  if (spreadcurve_cs01 == NULL){
  	std::cout << spreadcurve_cs01 << std::endl;
  	std::cout << "bad spreadcurve_cs01" << std::endl;
  }

  if (spreadcurve_cs01 != NULL){
    if (verbose){
  	  std::cout << spreadcurve_cs01 << std::endl;
  	  std::cout << "spreadcurve_cs01 OK!" << std::endl;
  	}
  }

  // dv01 spread_curve

  spreadcurve_dv01 = build_credit_spread_par_curve(
  			value_date_jpm
  			, zerocurve_dv01
  			, effective_date_jpm
  			, pointer_spreads
  			, pointer_tenors
  			, recovery_rate
  			, imm_dates.size()
  			, verbose);

  if (spreadcurve_dv01 == NULL){
  	std::cout << spreadcurve_dv01 << std::endl;
  	std::cout << "bad spreadcurve_dv01" << std::endl;
  }

  if (spreadcurve_dv01 != NULL){
    if (verbose){
  	  std::cout << spreadcurve_dv01 << std::endl;
  	  std::cout << "spreadcurve_dv01 OK!" << std::endl;
  	}
  }

  int is_dirty_price = 0;
  int is_clean_price = 1;

  // calculate price cds
  dirtypv = -calculate_cds_price(value_date_jpm
  , maturity_date_jpm
  , zerocurve
  , spreadcurve
  , accrual_start_date_jpm
  , recovery_rate
  , coupon_rate_in_basis_points
  , is_dirty_price
  , verbose);

  // calculate price cds
  cleanpv = -calculate_cds_price(value_date_jpm
  , maturity_date_jpm
  , zerocurve
  , spreadcurve
  , accrual_start_date_jpm
  , recovery_rate
  , coupon_rate_in_basis_points
  , is_clean_price
  , verbose);

  // compute accured interest
  ai = dirtypv - cleanpv;

   dirtypv_cs01 = -calculate_cds_price(value_date_jpm
  , maturity_date_jpm
  , zerocurve
  , spreadcurve_cs01
  , accrual_start_date_jpm
  , recovery_rate
  , coupon_rate_in_basis_points
  , is_dirty_price
  , verbose);

  dirtypv_dv01 = -calculate_cds_price(value_date_jpm
  , maturity_date_jpm
  , zerocurve_dv01
  , spreadcurve_dv01
  , accrual_start_date_jpm
  , recovery_rate
  , coupon_rate_in_basis_points
  , is_dirty_price
  , verbose);

  if (is_buy_protection){
  	credit_risk_direction_scale_factor = -1;
  }

  // push back result
  allinone_base.push_back(dirtypv * notional * credit_risk_direction_scale_factor);
  allinone_base.push_back(cleanpv * notional * credit_risk_direction_scale_factor);
  allinone_base.push_back(ai * notional );
  allinone_base.push_back((dirtypv_cs01 - dirtypv) * notional * credit_risk_direction_scale_factor);
  allinone_base.push_back((dirtypv_dv01 - dirtypv) * notional * credit_risk_direction_scale_factor);

// compute PVBP

  for(int r = 0; r < imm_dates.size(); r++){
     allinone_pvbp.push_back(
		  (calculate_cds_price(value_date_jpm
		  , tenors[r]
		  , zerocurve
		  , spreadcurve
		  , value_date_jpm
		  , recovery_rate
		  , 0.01
  		  , is_dirty_price
		  , verbose)
		  -
		  calculate_cds_price(value_date_jpm
		  , tenors[r]
		  , zerocurve
		  , spreadcurve
		  , value_date_jpm
		  , recovery_rate
		  , 0.01 + single_basis_point
		  , is_dirty_price
		  , verbose)) * credit_risk_direction_scale_factor
	  );
  }

  // roll down pv
  // move this into a C library function?

  for(int r = 0; r < spread_roll_tenors.size(); r++){
    spread_roll_expiries[r] = (char *)spread_roll_tenors[r].c_str();
  }

  pointer_roll_dates_jpm =  calculate_cds_roll_dates(maturity_date_jpm,
  	spread_roll_expiries,
  	spread_roll_tenors.size(),
  	verbose);

  for(int s=0; s < scenario_tenors.size(); s++){

	  vector <double> scenario_tenors_pvdirty;

	  // build a scenario spread curve
	  for(int r = 0; r < spread_rates.size(); r++){
	    // spread_cs01 = spread + spread * -0.1
		spreads_cs01[r] = spread_rates[r] + spread_rates[r]  * scenario_tenors[s]/100;
	  }

	  // similarly need a double *
	  pointer_spreads_cs01 = spreads_cs01;

	  // build spread curve
	  spreadcurve = build_credit_spread_par_curve(
				value_date_jpm
				, zerocurve
				, effective_date_jpm
				, pointer_spreads_cs01
				, pointer_tenors
				, recovery_rate
				, imm_dates.size()
				, verbose);

	  for(int r = 0; r < spread_roll_tenors.size(); r++){

		roll_pvclean = -calculate_cds_price(value_date_jpm
		  , pointer_roll_dates_jpm[r]
		  , zerocurve
		  , spreadcurve
		  , accrual_start_date_jpm
		  , recovery_rate
		  , coupon_rate_in_basis_points
		  , is_clean_price
		  , verbose);

		//roll_pvdirty = fabs(roll_pvdirty);
		scenario_tenors_pvdirty.push_back( roll_pvclean * notional * credit_risk_direction_scale_factor);

	  }

	  // push back entire matrix
	  allinone_roll.push_back(scenario_tenors_pvdirty);
  }
  
  // compute the par spread vector
  par_spread_pointer = calculate_cds_par_spread(value_date_jpm
		  , maturity_date_jpm
		  , zerocurve
		  , spreadcurve
		  , accrual_start_date_jpm
		  , recovery_rate
		  , coupon_rate_in_basis_points
		  , is_clean_price
		  , verbose
		  , expiries
  	      , swap_tenors.size());
  
  vector <double> par_spread_vector;
  for(int s=0; s < swap_tenors.size(); s++){
    par_spread_vector.push_back(par_spread_pointer[s]);  
  }
  
  int stop_s = clock();
  allinone_base.push_back((stop_s-start_s));

  // push back all vectors
  allinone.push_back(allinone_base);
  allinone.push_back(allinone_pvbp);
  allinone.push_back(par_spread_vector);

  for(int r = 0; r < allinone_roll.size(); r++){
	allinone.push_back(allinone_roll[r]);
  }
  
  // handle free of the curve objects via call to JpmcdsFreeSafe macro
  FREE(spreadcurve);
  FREE(spreadcurve_dv01);
  FREE(spreadcurve_cs01);
  FREE(zerocurve);
  FREE(zerocurve_dv01);
  FREE(pointer_roll_dates_jpm);


  return allinone;
}

/*
***************************************************************************
** compute all in one for a what if cds position.
***************************************************************************
*/

vector< vector<double> > cds_all_in_one_exclude_ir_tenor_dates (
 string trade_date,					/* (I) trade date of cds as DD/MM/YYYY */
 string effective_date,				/* (I) effective date of cds as DD/MM/YYYY */
 string maturity_date,				/* (I) maturity date of cds as DD/MM/YYYY */
 string value_date,					/* (I) date to value the cds DD/MM/YYYY */
 string accrual_start_date,			/* (I) accrual start date of cds as DD/MM/YYYY */
 double	recovery_rate,				/* (I) recover rate of the curve in basis points */
 double coupon_rate,				/* (I) CouponRate (e.g. 0.05 = 5% = 500bp) */
 double notional,					/* (I) Notional MM */
 int is_buy_protection,				/* (I) direction of credit risk */
 vector<double> swap_rates, 		/* (I) swap rates */
 vector<string> swap_tenors,		/* (I) swap tenors "1M", "2M" */
 vector<double> spread_rates,		/* (I) spread spreads */
 vector<string> spread_tenors,		/* (I) spread tenors "6M", "1Y" */
 vector<string> spread_roll_tenors, /* (I) spread roll tenors */
 vector<string> imm_dates,			/* (I) imm dates */
 vector<double> scenario_tenors,	/* (I) spread tenors -100, -90, -80, -70 ... */
 int verbose						/* (I) output message text */
)
{

  int start_s = clock();

  // used in risk calculations
  double single_basis_point = 0.0001;

  TDate trade_date_jpm, effective_date_jpm, maturity_date_jpm,
  accrual_start_date_jpm, value_date_jpm;

  // empty curve pointers
  TCurve *zerocurve = NULL;
  TCurve *zerocurve_dv01 = NULL;
  TCurve *spreadcurve = NULL;
  TCurve *spreadcurve_cs01 = NULL;
  TCurve *spreadcurve_dv01 = NULL;

  // discount
  double rates[swap_rates.size()];
  double rates_dv01[swap_rates.size()];
  char *expiries[swap_rates.size()];

  // credit spread
  double spreads[spread_rates.size()];
  double spreads_cs01[spread_rates.size()];
  double *pointer_spreads;
  double *pointer_spreads_cs01;
  long int tenors[imm_dates.size()];
  long int *pointer_tenors;

  char *spread_roll_expiries[spread_roll_tenors.size()];

  // numeric values
  double coupon_rate_in_basis_points = coupon_rate/10000.0;
  double upfrontcharge;
  double dirtypv;
  double cleanpv;
  double ai;
  double dirtypv_cs01;
  double dirtypv_dv01;

  // outer return vector
  vector < vector<double> > allinone;

  // inner return vector
  vector <double> allinone_base;
  vector <double> allinone_pvbp;
  vector < vector<double> > allinone_roll;

  // assumes sell protection default
  double credit_risk_direction_scale_factor = 1;

  // jpm roll dates
  TDate *pointer_roll_dates_jpm;
  double roll_pvclean;

  /* TODO: check swap_rates.size() == imm_dates.size() with assert */
  /* TODO: check swap_rates.size() == imm_dates.size() with assert */

  /////////////////////////////
  // parse char* to jpm dates
  /////////////////////////////

  trade_date_jpm = parse_string_ddmmyyyy_to_jpmcdsdate(trade_date);
  effective_date_jpm = parse_string_ddmmyyyy_to_jpmcdsdate(effective_date);
  maturity_date_jpm = parse_string_ddmmyyyy_to_jpmcdsdate(maturity_date);
  accrual_start_date_jpm = parse_string_ddmmyyyy_to_jpmcdsdate(accrual_start_date);
  value_date_jpm = parse_string_ddmmyyyy_to_jpmcdsdate(value_date);

  if (verbose == 1){
	  std::cout << "value_date_jpm " << value_date_jpm << std::endl;
	  std::cout << "trade_date_jpm " << trade_date_jpm << std::endl;
	  std::cout << "effective_date_jpm " << effective_date_jpm << std::endl;
	  std::cout << "accrual_start_date_jpm " << accrual_start_date_jpm << std::endl;
	  std::cout << "maturity_date_jpm " << maturity_date_jpm << std::endl;
  }

  /////////////////////////////
  // bootstrap interest rate curve
  /////////////////////////////

  for(int r = 0; r < swap_rates.size(); r++){
    rates[r] = swap_rates[r];
    rates_dv01[r] = swap_rates[r] + single_basis_point;
  }

  for(int r = 0; r < swap_tenors.size(); r++){
    expiries[r] = (char *)swap_tenors[r].c_str();
  }

  // bootstrap discount curve
  zerocurve = build_zero_interest_rate_curve(value_date_jpm
		  , rates
		  , expiries
		  , verbose);

  zerocurve_dv01 = build_zero_interest_rate_curve(value_date_jpm
		  , rates_dv01
		  , expiries
		  , verbose);

  /////////////////////////////
  // bootstrap spread curve
  /////////////////////////////

  // parse imm dates into jpm string format
  for(int r = 0; r < imm_dates.size(); r++){
    tenors[r] = parse_string_ddmmyyyy_to_jpmcdsdate(imm_dates[r]);
    if (verbose){
    	std::cout << r << " imm tenor " << tenors[r] << std::endl;
    }
  }

  // need a pointer to array of long int
  pointer_tenors = tenors;

  for(int r = 0; r < spread_rates.size(); r++){
    spreads[r] = spread_rates[r];
    spreads_cs01[r] = spread_rates[r] + single_basis_point;
    if (verbose){
      std::cout << r << " " << spreads_cs01[r] << std::endl;
    }
  }

  // similarly need a double *
  pointer_spreads = spreads;
  pointer_spreads_cs01 = spreads_cs01;

  if (verbose){
    std::cout << "imm_dates.size() " << imm_dates.size() << std::endl;
  }

  // build spread curve
  spreadcurve = build_credit_spread_par_curve(
  			value_date_jpm
  			, zerocurve
  			, effective_date_jpm
  			, pointer_spreads
  			, pointer_tenors
  			, recovery_rate
  			, imm_dates.size()
  			, verbose);

  if (spreadcurve == NULL){
  	std::cout << spreadcurve << std::endl;
  	std::cout << "bad spreadcurve" << std::endl;
  }

  if (spreadcurve != NULL){
    if (verbose){
  	  std::cout << spreadcurve << std::endl;
  	  std::cout << "spreadcurve OK!" << std::endl;
  	}
  }

  // cs01_spread_curve

  // build spread curve
  spreadcurve_cs01 = build_credit_spread_par_curve(
  			value_date_jpm
  			, zerocurve
  			, effective_date_jpm
  			, pointer_spreads_cs01
  			, pointer_tenors
  			, recovery_rate
  			, imm_dates.size()
  			, verbose);

  if (spreadcurve_cs01 == NULL){
  	std::cout << spreadcurve_cs01 << std::endl;
  	std::cout << "bad spreadcurve_cs01" << std::endl;
  }

  if (spreadcurve_cs01 != NULL){
    if (verbose){
  	  std::cout << spreadcurve_cs01 << std::endl;
  	  std::cout << "spreadcurve_cs01 OK!" << std::endl;
  	}
  }

  // dv01 spread_curve

  spreadcurve_dv01 = build_credit_spread_par_curve(
  			value_date_jpm
  			, zerocurve_dv01
  			, effective_date_jpm
  			, pointer_spreads
  			, pointer_tenors
  			, recovery_rate
  			, imm_dates.size()
  			, verbose);

  if (spreadcurve_dv01 == NULL){
  	std::cout << spreadcurve_dv01 << std::endl;
  	std::cout << "bad spreadcurve_dv01" << std::endl;
  }

  if (spreadcurve_dv01 != NULL){
    if (verbose){
  	  std::cout << spreadcurve_dv01 << std::endl;
  	  std::cout << "spreadcurve_dv01 OK!" << std::endl;
  	}
  }

  // calculate upfront on cds
  //TODO: correctly pass trade details here!
  //upfrontcharge = calculate_upfront_charge(zerocurve, coupon_rate, verbose);

  int is_dirty_price = 0;
  int is_clean_price = 1;

  // calculate price cds
  dirtypv = -calculate_cds_price(value_date_jpm
  , maturity_date_jpm
  , zerocurve
  , spreadcurve
  , accrual_start_date_jpm
  , recovery_rate
  , coupon_rate_in_basis_points
  , is_dirty_price
  , verbose);

  // calculate price cds
  cleanpv = -calculate_cds_price(value_date_jpm
  , maturity_date_jpm
  , zerocurve
  , spreadcurve
  , accrual_start_date_jpm
  , recovery_rate
  , coupon_rate_in_basis_points
  , is_clean_price
  , verbose);

  // compute accured interest 
  ai = dirtypv - cleanpv;

  dirtypv_cs01 = -calculate_cds_price(value_date_jpm
  , maturity_date_jpm
  , zerocurve
  , spreadcurve_cs01
  , accrual_start_date_jpm
  , recovery_rate
  , coupon_rate_in_basis_points
  , is_dirty_price
  , verbose);

  dirtypv_dv01 = -calculate_cds_price(value_date_jpm
  , maturity_date_jpm
  , zerocurve_dv01
  , spreadcurve_dv01
  , accrual_start_date_jpm
  , recovery_rate
  , coupon_rate_in_basis_points
  , is_dirty_price
  , verbose);

  if (is_buy_protection){
  	credit_risk_direction_scale_factor = -1;
  }

  // manage sign separately based on trade direction
  //dirtypv = fabs(dirtypv);  
  //dirtypv_cs01 = fabs(dirtypv_cs01);
  //dirtypv_dv01 = fabs(dirtypv_dv01);

  if (verbose == 1){
    std::cout << "credit_risk_direction_scale_factor " << credit_risk_direction_scale_factor << std::endl;
    std::cout << "dirtypv * notional " << dirtypv * notional  << std::endl;
    std::cout << "dirtypv " << dirtypv << std::endl;
    std::cout << "cleanpv " << cleanpv << std::endl;
	std::cout << "ai " << ai << std::endl;
	
	printf("dirtypv %.14f\tcleanpv %.14f\tai %.14f\tdirtypv_cs01 %.14f\n", dirtypv, cleanpv, ai, dirtypv_cs01);
	
    std::cout << "pvdirty (scaled) " << dirtypv * notional * credit_risk_direction_scale_factor << std::endl;
  }

  if (verbose){
    std::cout << "cs01 " << dirtypv_cs01 - dirtypv << std::endl;
  }

  // push back result
  allinone_base.push_back(dirtypv * notional * credit_risk_direction_scale_factor);
  allinone_base.push_back(cleanpv * notional * credit_risk_direction_scale_factor);
  allinone_base.push_back(ai);
  allinone_base.push_back((dirtypv_cs01 - dirtypv) * notional);
  allinone_base.push_back((dirtypv_dv01 - dirtypv) * notional);

  //allinone.push_back(upfrontcharge);

  // compute PVBP

  for(int r = 0; r < imm_dates.size(); r++){
     allinone_pvbp.push_back(
		  (calculate_cds_price(value_date_jpm
		  , tenors[r]
		  , zerocurve
		  , spreadcurve
		  , value_date_jpm
		  , recovery_rate
		  , 0.01
  		  , is_dirty_price
		  , verbose)
		  -
		  calculate_cds_price(value_date_jpm
		  , tenors[r]
		  , zerocurve
		  , spreadcurve
		  , value_date_jpm
		  , recovery_rate
		  , 0.01 + single_basis_point
		  , is_dirty_price
		  , verbose)) * credit_risk_direction_scale_factor
	  );
  }

  // roll down pv
  // move this into a C library function?

  for(int r = 0; r < spread_roll_tenors.size(); r++){
    spread_roll_expiries[r] = (char *)spread_roll_tenors[r].c_str();
  }

  //pointer_roll_dates_jpm =  calculate_cds_roll_dates(value_date_jpm,
  //spread_roll_expiries,
  //spread_roll_tenors.size(),
  //verbose);

  pointer_roll_dates_jpm =  calculate_cds_roll_dates(maturity_date_jpm,
  	spread_roll_expiries,
  	spread_roll_tenors.size(),
  	verbose);

  for(int s=0; s < scenario_tenors.size(); s++){  
  	  
	  vector <double> scenario_tenors_pvdirty;
	  
	  // build a scenario spread curve	  
	  for(int r = 0; r < spread_rates.size(); r++){		
	    // spread_cs01 = spead + spread * -0.1
		spreads_cs01[r] = spread_rates[r] + spread_rates[r]  * scenario_tenors[s]/100;		
	  }

	  // similarly need a double *	  
	  pointer_spreads_cs01 = spreads_cs01;
	  
	  // build spread curve
	  spreadcurve = build_credit_spread_par_curve(
				value_date_jpm
				, zerocurve
				, effective_date_jpm
				, pointer_spreads_cs01
				, pointer_tenors
				, recovery_rate
				, imm_dates.size()
				, verbose);	 
  
	  for(int r = 0; r < spread_roll_tenors.size(); r++){

		roll_pvclean = -calculate_cds_price(value_date_jpm
		  , pointer_roll_dates_jpm[r]
		  , zerocurve
		  , spreadcurve
		  , accrual_start_date_jpm
		  , recovery_rate
		  , coupon_rate_in_basis_points
		  , is_clean_price
		  , verbose);

		//roll_pvdirty = fabs(roll_pvdirty);
		scenario_tenors_pvdirty.push_back(roll_pvclean * notional * credit_risk_direction_scale_factor);

	  }
	  
	  // push back entire matrix
	  allinone_roll.push_back(scenario_tenors_pvdirty);
  }

  int stop_s = clock();
  allinone_base.push_back((stop_s-start_s));

  // push back all vectors
  allinone.push_back(allinone_base);
  allinone.push_back(allinone_pvbp);
  
  for(int r = 0; r < allinone_roll.size(); r++){		
	allinone.push_back(allinone_roll[r]);
  }

  // handle free of the curve objects via call to JpmcdsFreeSafe macro
  FREE(spreadcurve);
  FREE(spreadcurve_dv01);
  FREE(spreadcurve_cs01);
  FREE(zerocurve);
  FREE(zerocurve_dv01);
  FREE(pointer_roll_dates_jpm);

  return allinone;
}

vector<double> average (vector< vector<double> > i_matrix) {

  // compute average of each row..
  vector <double> averages; 
  for (int r = 0; r < i_matrix.size(); r++){
    double rsum = 0.0;
    double ncols= i_matrix[r].size();
    for (int c = 0; c< i_matrix[r].size(); c++){
      rsum += i_matrix[r][c];
    }
    averages.push_back(rsum/ncols);    
  }
  std::cout << "factoral " << factoral(5) << std::endl;
  return averages;
};


int n=0;
Callback * callback = NULL;

void Callback::run(int n){
    std::cout << "This print from C++: n = " << n << std::endl;
}

void setCallback(Callback * cb){
    callback = cb;
}

void doSomeWithCallback(){
    if(callback == NULL){
        std::cout << "Must set callback first!" << std::endl;
    }else{
        callback->run(n++);
    }
}
