/*=========================================================================
//程序名称:     f_caai_unit
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:
//修改日期:
//函数说明：吨钢定额消耗
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件
BM2_FUNCTION_EXPORT
double f_caai_getprice(const CString& mat_code, const CString& price_terms, const CString& dept_code, CDbConnection * conn);
int f_caai_unit(CString stats_period, CString dept_code, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;
	CDecimal price = 0;

	CString sqlstr = "";
	CString sqlstr1 = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);

	CModel tcaai03("TCAAI03");
	CModel tcaac10("TCAAC10");

	try
	{
		sqlstr = " delete from tcaai04"
			" where 1=1"
			" and data_from = 'unit'"
			" AND DEPT_CODE= @dept_code"
			" AND stats_period = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		if (dept_code == "S1")
		{
			//特殊处理的：差旅费rcb0024,rcb0027维修费,铁水包承包（06020005/06020030）, 钢包承包（06020002/06020001）, 引流砂承包（06020025/06020024）
			sqlstr = "select cost_center sub_backlog_code,mat_code,unit_cost,unit_cost2"
				" from (select cost_center ,mat_code,SUM(decode(type_code,'GJ',unit_cost,0)) unit_cost,SUM(decode(type_code,'CJ',unit_cost,0)) unit_cost2"
				" from tcaac10"
				" where 1=1 "				
				" and sg_sign = ' '"
				" and mat_code not in ('rcb0024','rcb0027','06020005','06020030','06020002','06020001','06020025','06020024')"
				" AND DEPT_CODE= @dept_code"
				" GROUP BY cost_center ,mat_code "
				")"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
					" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
					" SELECT @datenow, dept_code, stats_period, cost_center, product_code, @mat_code, 0, wt*@unit_cost,wt*@unit_cost2, wt*@unit_cost,wt*@unit_cost2"
					" , prod_date, prod_shift_group, prod_shift_no, @sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
					" from tcaai03"
					" where 1=1"
					" AND dept_code = @dept_code"
					" and sub_backlog_code = 'C'"
					" AND stats_period = @stats_period"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("datenow", datenow);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
				cmd_inq_1.Parameters.Set("unit_cost", cmd_inq.GetDecimal(3));
				cmd_inq_1.Parameters.Set("unit_cost2", cmd_inq.GetDecimal(4));
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}
			cmd_inq.Close();

			//其他的需要分钢种规格
			sqlstr = " select distinct product_code,sg_sign,mat_thick"
				" from tcaai03 t1 "
				" where 1=1"
				" and sub_backlog_code = 'C' "
				" AND dept_code = @dept_code"
				" AND stats_period = @stats_period"
				;
			Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				//铁水包承包（06020030/06020005）
				sqlstr = " select mat_code,cost_center, SUM(decode(type_code,'GJ',unit_cost,0)) unit_cost, SUM(decode(type_code,'CJ',unit_cost,0)) unit_cost2"
					" from tcaac10"
					" where sg_sign = @sg_sign"
					" and mat_code = '06020030'"
					" AND dept_code = @dept_code"
					" GROUP BY mat_code,cost_center"
					;
				cmd_inq_sub.SetCommandText(sqlstr);
				cmd_inq_sub.Parameters.Set("dept_code", dept_code);
				cmd_inq_sub.Parameters.Set("stats_period", stats_period);
				cmd_inq_sub.Parameters.Set("sg_sign", cmd_inq.GetString(2));
				cmd_inq_sub.ExecuteReader();
				if (cmd_inq_sub.Read())
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, dept_code, stats_period, cost_center, product_code, @mat_code, 0, round(wt*@unit_cost,6),round(wt*@unit_cost2,6), round(wt*@unit_cost,6),round(wt*@unit_cost2,6)"
						" , prod_date, prod_shift_group, prod_shift_no, @sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03"
						" where 1=1"
						" and product_code=@product_code"
						" and mat_thick = @mat_thick"
						" and sg_sign = @sg_sign"
						" AND dept_code = @dept_code"
						" and sub_backlog_code = 'C'"
						" AND stats_period = @stats_period"
						;
					Log::Trace("", "", "06020030sqlstr = [{0}],sg_sign={1},mat_thick={2},product_code={3},sub_backlog_code={4},dept_code={5},stats_period={6}", sqlstr, cmd_inq.GetString(2), cmd_inq.GetDecimal(3), cmd_inq.GetString(1), cmd_inq_sub.GetString(2), dept_code, stats_period);
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq_sub.GetString(2));
					cmd_inq_1.Parameters.Set("mat_code", cmd_inq_sub.GetString(1));
					cmd_inq_1.Parameters.Set("unit_cost", cmd_inq_sub.GetDecimal(3));
					cmd_inq_1.Parameters.Set("unit_cost2", cmd_inq_sub.GetDecimal(4));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
				else
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, t1.dept_code, t1.stats_period, t1.cost_center, t1.product_code, mat_code, 0, round(wt*unit_cost,6), round(wt*unit_cost2,6),  round(wt*unit_cost,6), round(wt*unit_cost2,6)"
						" , prod_date, prod_shift_group, prod_shift_no, @sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03 t1 "
						" left join (SELECT COST_CENTER,MAT_CODE,SUM(decode(type_code,'GJ',unit_cost,0)) unit_cost, SUM(decode(type_code,'CJ',unit_cost,0)) unit_cost2 FROM tcaac10 where mat_code ='06020005' and sg_sign =@sg_sign and dept_code  =@dept_code group by COST_CENTER,MAT_CODE) t2 on   t2.COST_CENTER=@sub_backlog_code "
						" where 1=1"
						" and exists (select 1 from tcaac10 t2 where   t2.COST_CENTER=@sub_backlog_code  and t2.mat_code ='06020005' and sg_sign =@sg_sign and dept_code  =@dept_code)"
						" and t1.product_code = @product_code"
						" and t1.sub_backlog_code = 'C'"
						" and t1.mat_thick = @mat_thick"
						" and t1.sg_sign =@sg_sign"
						" AND t1.dept_code = @dept_code"
						" AND t1.stats_period = @stats_period"
						;
					Log::Trace("", "", "06020005sqlstr = [{0}],sg_sign={1},mat_thick={2},product_code={3},sub_backlog_code={4},dept_code={5},stats_period={6}", sqlstr, cmd_inq.GetString(2), cmd_inq.GetDecimal(3), cmd_inq.GetString(1), cmd_inq_sub.GetString(2), dept_code, stats_period);
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sub_backlog_code", "L");
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
				cmd_inq_sub.Close();

				//钢包承包（06020002 / 06020001）
				sqlstr = " select mat_code,cost_center, SUM(decode(type_code,'GJ',unit_cost,0)) unit_cost, SUM(decode(type_code,'CJ',unit_cost,0)) unit_cost2"
					" from tcaac10"
					" where sg_sign = @sg_sign"
					" and mat_code = '06020002'"
					" AND dept_code = @dept_code"
					" group by mat_code,cost_center"
					;
				cmd_inq_sub.SetCommandText(sqlstr);
				cmd_inq_sub.Parameters.Set("dept_code", dept_code);
				cmd_inq_sub.Parameters.Set("stats_period", stats_period);
				cmd_inq_sub.Parameters.Set("sg_sign", cmd_inq.GetString(2));
				cmd_inq_sub.ExecuteReader();
				if (cmd_inq_sub.Read())
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, dept_code, stats_period, cost_center, product_code, @mat_code, 0, round(wt*@unit_cost,6),round(wt*@unit_cost2,6), round(wt*@unit_cost,6),round(wt*@unit_cost2,6)"
						" , prod_date, prod_shift_group, prod_shift_no, @sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03"
						" where 1=1"
						" and product_code = @product_code"
						" and mat_thick = @mat_thick"
						" and sg_sign = @sg_sign"
						" AND dept_code = @dept_code"
						" and sub_backlog_code = 'C'"
						" AND stats_period = @stats_period"
						;
					Log::Trace("", "", "06020002sqlstr = [{0}],sg_sign={1},mat_thick={2},product_code={3},sub_backlog_code={4},dept_code={5},stats_period={6}", sqlstr, cmd_inq.GetString(2), cmd_inq.GetDecimal(3), cmd_inq.GetString(1), cmd_inq_sub.GetString(2), dept_code, stats_period);
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq_sub.GetString(2));
					cmd_inq_1.Parameters.Set("mat_code", cmd_inq_sub.GetString(1));
					cmd_inq_1.Parameters.Set("unit_cost", cmd_inq_sub.GetDecimal(3));
					cmd_inq_1.Parameters.Set("unit_cost", cmd_inq_sub.GetDecimal(4));
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
				else
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, t1.dept_code, t1.stats_period, t1.cost_center, t1.product_code, mat_code, 0, round(wt*unit_cost,6),round(wt*unit_cost2,6), round(wt*unit_cost,6),round(wt*unit_cost2,6)"
						" , t1.prod_date, prod_shift_group, prod_shift_no, @sub_backlog_code, t1.equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03 t1 "
						" left join (SELECT COST_CENTER,MAT_CODE,SUM(decode(type_code,'GJ',unit_cost,0)) unit_cost, SUM(decode(type_code,'CJ',unit_cost,0)) unit_cost2 FROM tcaac10 where mat_code ='06020001' and sg_sign = @sg_sign and dept_code = @dept_code group by COST_CENTER,MAT_CODE) t2 on   t2.COST_CENTER=@sub_backlog_code "						
						" where 1=1"
						" and exists (select 1 from tcaac10 t2 where  t2.COST_CENTER=@sub_backlog_code  and t2.mat_code ='06020001' and sg_sign =@sg_sign and dept_code  =@dept_code) "
						" and t1.product_code = @product_code"
						" and t1.mat_thick = @mat_thick"
						" and t1.sub_backlog_code = 'C'"
						" and t1.sg_sign =@sg_sign"
						" AND t1.dept_code = @dept_code"
						" AND t1.stats_period = @stats_period"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sub_backlog_code", "L");
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
				cmd_inq_sub.Close();

				//引流砂承包（06020025 / 06020024）
				sqlstr = " select mat_code,cost_center, SUM(decode(type_code,'GJ',unit_cost,0)) unit_cost, SUM(decode(type_code,'CJ',unit_cost,0)) unit_cost2"
					" from tcaac10"
					" where sg_sign = @sg_sign"
					" and mat_code = '06020025'"
					" AND dept_code = @dept_code"
					" group by mat_code,cost_center"
					;
				cmd_inq_sub.SetCommandText(sqlstr);
				cmd_inq_sub.Parameters.Set("dept_code", dept_code);
				cmd_inq_sub.Parameters.Set("stats_period", stats_period);
				cmd_inq_sub.Parameters.Set("sg_sign", cmd_inq.GetString(2));
				cmd_inq_sub.ExecuteReader();
				if (cmd_inq_sub.Read())
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, dept_code, stats_period, cost_center, product_code, @mat_code, 0, round(wt*@unit_cost,6),round(wt*@unit_cost2,6), round(wt*@unit_cost,6),round(wt*@unit_cost2,6)"
						" , prod_date, prod_shift_group, prod_shift_no, @sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03"
						" where 1=1"
						" and product_code = @product_code"
						" and sg_sign = @sg_sign"
						" and mat_thick = @mat_thick"
						" AND dept_code = @dept_code"
						" and sub_backlog_code = 'C'"
						" AND stats_period = @stats_period"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq_sub.GetString(2));
					cmd_inq_1.Parameters.Set("mat_code", cmd_inq_sub.GetString(1));
					cmd_inq_1.Parameters.Set("unit_cost", cmd_inq_sub.GetDecimal(3));
					cmd_inq_1.Parameters.Set("unit_cost", cmd_inq_sub.GetDecimal(4));
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
				else
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, t1.dept_code, t1.stats_period, t1.cost_center, t1.product_code, mat_code, 0, round(wt*unit_cost,6),round(wt*unit_cost2,6), round(wt*unit_cost,6),round(wt*unit_cost2,6)"
						" , prod_date, prod_shift_group, prod_shift_no, @sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03 t1 "
						" left join (SELECT COST_CENTER,MAT_CODE,SUM(decode(type_code,'GJ',unit_cost,0)) unit_cost, SUM(decode(type_code,'CJ',unit_cost,0)) unit_cost2 FROM tcaac10 where mat_code ='06020024' and sg_sign = @sg_sign and dept_code = @dept_code group by COST_CENTER,MAT_CODE) t2 on   t2.COST_CENTER=@sub_backlog_code "
						" where 1=1"
						" and exists (select 1 from tcaac10 t2 where   t2.COST_CENTER=@sub_backlog_code  and t2.mat_code ='06020024' and sg_sign =@sg_sign and dept_code  =@dept_code)"
						" and t1.mat_thick = @mat_thick"
						" and t1.product_code = @product_code"
						" and t1.sub_backlog_code = 'C'"
						" and t1.sg_sign =@sg_sign"
						" AND t1.dept_code = @dept_code"
						" AND t1.stats_period = @stats_period"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sub_backlog_code", "L");
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
				cmd_inq_sub.Close();

				//取差旅费rcb0024,rcb0027维修费
				if (cmd_inq.GetString(2) == "45"&& cmd_inq.GetString(1) == "DA"&&cmd_inq.GetDecimal(3)<=6.5) //如果是45钢并且是线材
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, t1.dept_code, t1.stats_period, t1.cost_center, t1.product_code, mat_code, 0, round(wt*unit_cost,6),round(wt*unit_cost,6),round(wt*unit_cost,6),round(wt*unit_cost,6)"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03 t1 "
						" left join tcaac10 t2 on   t1.sub_backlog_code=t2.COST_CENTER  and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ' and prod_code = 'DA' AND t2.MAT_THICK = 6.5 and t2.mat_code in ('rcb0024')"
						" where 1=1"
						" and exists (select 1 from tcaac10 t2 where  t1.sub_backlog_code=t2.COST_CENTER  and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ' and prod_code = 'DA' AND t2.MAT_THICK = 6.5 and t2.mat_code in ('rcb0024'))"
						" and t1.product_code = @product_code"
						" and t1.mat_thick = @mat_thick"
						" and t1.sub_backlog_code = 'C'"
						" and t1.sg_sign =@sg_sign"
						" AND t1.dept_code = @dept_code"
						" AND t1.stats_period = @stats_period"
						;
					Log::Trace("", "", "45DAsqlstr = [{0}]", sqlstr);
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();

					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, t1.dept_code, t1.stats_period, t1.cost_center, t1.product_code, mat_code, 0, round(wt*unit_cost,6),round(wt*unit_cost,6), round(wt*unit_cost,6),round(wt*unit_cost,6)"
						" , prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03 t1 "
						" left join tcaac10 t2 on  t1.sub_backlog_code=t2.COST_CENTER   and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ' and prod_code = 'DA' AND t2.MAT_THICK = 6.5 and t2.mat_code in ('rcb0027')"
						" where 1=1"
						" and exists (select 1 from tcaac10 t2 where  t1.sub_backlog_code=t2.COST_CENTER   and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ' and prod_code = 'DA' AND t2.MAT_THICK = 6.5 and t2.mat_code in ('rcb0027'))"
						" and t1.product_code = @product_code"
						" and t1.mat_thick = @mat_thick"
						" and t1.sub_backlog_code = 'C'"
						" and t1.sg_sign =@sg_sign"
						" AND t1.dept_code = @dept_code"
						" AND t1.stats_period = @stats_period"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
				else if (cmd_inq.GetString(2) == "45"&& cmd_inq.GetString(1) == "DA"&&cmd_inq.GetDecimal(3) > 6.5)
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, t1.dept_code, t1.stats_period, t1.cost_center, t1.product_code, mat_code, 0, round(wt*unit_cost,6),round(wt*unit_cost,6),round(wt*unit_cost,6),round(wt*unit_cost,6)"
						" , prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03 t1 "
						" left join tcaac10 t2 on  t1.sub_backlog_code=t2.COST_CENTER   and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ' and prod_code = 'DA' AND t2.MAT_THICK = 6.55 and t2.mat_code in ('rcb0024')"
						" where 1=1"
						" and exists (select 1 from tcaac10 t2 where   t1.sub_backlog_code=t2.COST_CENTER  and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ' and prod_code = 'DA' AND t2.MAT_THICK = 6.55 and t2.mat_code in ('rcb0024'))"
						" and t1.product_code = @product_code"
						" and t1.mat_thick = @mat_thick"
						" and t1.sub_backlog_code = 'C'"
						" and t1.sg_sign =@sg_sign"
						" AND t1.dept_code = @dept_code"
						" AND t1.stats_period = @stats_period"
						;
					Log::Trace("", "", "45 6.5sqlstr = [{0}]", sqlstr);
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();

					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, t1.dept_code, t1.stats_period, t1.cost_center, t1.product_code, mat_code, 0, round(wt*unit_cost,6),round(wt*unit_cost,6),round(wt*unit_cost,6),round(wt*unit_cost,6)"
						" , prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03 t1 "
						" left join tcaac10 t2 on  t1.sub_backlog_code=t2.COST_CENTER   and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ' and prod_code = 'DA' AND t2.MAT_THICK = 6.55 and t2.mat_code in ('rcb0027')"
						" where 1=1"
						" and exists (select 1 from tcaac10 t2 where  t1.sub_backlog_code=t2.COST_CENTER   and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ' and prod_code = 'DA' AND t2.MAT_THICK = 6.55 and t2.mat_code in ('rcb0027'))"
						" and t1.product_code = @product_code"
						" and t1.mat_thick = @mat_thick"
						" and t1.sub_backlog_code = 'C'"
						" and t1.sg_sign =@sg_sign"
						" AND t1.dept_code = @dept_code"
						" AND t1.stats_period = @stats_period"
						;
					//Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
				else if (cmd_inq.GetString(2) == "55" || (cmd_inq.GetString(2) == "45"&&cmd_inq.GetString(1) == "BA"))
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, t1.dept_code, t1.stats_period, t1.cost_center, t1.product_code, t2.mat_code, 0, round(wt*unit_cost,6),round(wt*unit_cost,6), round(wt*unit_cost,6),round(wt*unit_cost,6)"
						" , prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03 t1 "
						" left join tcaac10 t2 on  t1.sub_backlog_code=t2.COST_CENTER   and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ' and t1.product_code = t2.prod_code  and t2.mat_code in ('rcb0024')"
						" where 1=1"
						" and exists (select 1 from tcaac10 t2 where  t1.sub_backlog_code=t2.COST_CENTER   and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ' and t1.product_code = t2.prod_code  and t2.mat_code in ('rcb0024'))"
						" and t1.product_code = @product_code"
						" and t1.mat_thick = @mat_thick"
						" and t1.sub_backlog_code = 'C'"
						" and t1.sg_sign =@sg_sign"
						" AND t1.dept_code = @dept_code"
						" AND t1.stats_period = @stats_period"
						;
					Log::Trace("", "", "55sqlstr = [{0}]", sqlstr);
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();

					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, t1.dept_code, t1.stats_period, t1.cost_center, t1.product_code, mat_code, 0, round(wt*unit_cost,6),round(wt*unit_cost,6),round(wt*unit_cost,6),round(wt*unit_cost,6)"
						" , prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03 t1 "
						" left join tcaac10 t2 on  t1.sub_backlog_code=t2.COST_CENTER   and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ' and t1.product_code = t2.prod_code  and t2.mat_code in ('rcb0027')"
						" where 1=1"
						" and exists (select 1 from tcaac10 t2 where t1.sub_backlog_code=t2.COST_CENTER  and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ' and t1.product_code = t2.prod_code  and t2.mat_code in ('rcb0027'))"
						" and t1.product_code = @product_code"
						" and t1.mat_thick = @mat_thick"
						" and t1.sub_backlog_code = 'C'"
						" and t1.sg_sign =@sg_sign"
						" AND t1.dept_code = @dept_code"
						" AND t1.stats_period = @stats_period"
						;
					//Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
				else
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, t1.dept_code, t1.stats_period, t1.cost_center, t1.product_code, mat_code, 0, round(wt*unit_cost,6),round(wt*unit_cost,6), round(wt*unit_cost,6),round(wt*unit_cost,6)"
						" , prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, t1.equ_no, t1.sg_sign, t1.mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03 t1 "
						" left join tcaac10 t2 on  t1.sub_backlog_code=t2.COST_CENTER   and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ'  and t2.mat_code in ('rcb0024')"
						" where 1=1"
						" and exists (select 1 from  tcaac10 t2 where  t1.sub_backlog_code=t2.COST_CENTER   and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ'  and t2.mat_code in ('rcb0024') )"
						" and t1.product_code = @product_code"
						" and t1.mat_thick = @mat_thick"
						" and t1.sub_backlog_code = 'C'"
						" and t1.sg_sign =@sg_sign"
						" AND t1.dept_code = @dept_code"
						" AND t1.stats_period = @stats_period"
						;
					//Log::Trace("", "", "sqlstr = [{0}],sg_sign={1},mat_thick={2},product_code={3},sub_backlog_code={4},dept_code={5},stats_period={6}", sqlstr, cmd_inq.GetString(2), cmd_inq.GetDecimal(3), cmd_inq.GetString(1), cmd_inq_sub.GetString(2), dept_code, stats_period);
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();

					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2,std_amt_1,std_amt_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, t1.dept_code, t1.stats_period, t1.cost_center, t1.product_code, mat_code, 0, round(wt*unit_cost,6),round(wt*unit_cost,6),round(wt*unit_cost,6),round(wt*unit_cost,6)"
						" , prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, t1.equ_no, t1.sg_sign, t1.mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,'unit' "
						" from tcaai03 t1 "
						" left join tcaac10 t2 on  t1.sub_backlog_code=t2.COST_CENTER   and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ'  and t2.mat_code in ('rcb0027')"
						" where 1=1"
						" and exists (select 1 from  tcaac10 t2 where  t1.sub_backlog_code=t2.COST_CENTER   and t1.sg_sign =t2.sg_sign and t2.type_code = 'GJ'  and t2.mat_code in ('rcb0027') )"
						" and t1.product_code = @product_code"
						" and t1.mat_thick = @mat_thick"
						" and t1.sub_backlog_code = 'C'"
						" and t1.sg_sign =@sg_sign"
						" AND t1.dept_code = @dept_code"
						" AND t1.stats_period = @stats_period"
						;
					//Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("dept_code", dept_code);
					cmd_inq_1.Parameters.Set("stats_period", stats_period);			
					cmd_inq_1.Parameters.Set("sg_sign", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("mat_thick", cmd_inq.GetDecimal(3));
					cmd_inq_1.Parameters.Set("product_code", cmd_inq.GetString(1));
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
			}
			cmd_inq.Close();


			
		}
		else
		{
		
		//因为关联数据项太多无法直接一条sql执行，改为分类执行
		sqlstr = " select DEPT_CODE, STATS_PERIOD, COST_CENTER,PRODUCT_CODE,QTY,WT"
			" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5 "
			" from tcaai03"
			" WHERE 1=1"
			" AND dept_code = @dept_code"
			" AND stats_period = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			tcaai03.Reset();
			cmd_inq.Fetch(tcaai03);
			tcaac10["DEPT_CODE"] = tcaai03["DEPT_CODE"].ToString();
			tcaac10["COST_CENTER"] = tcaai03["COST_CENTER"].ToString();
			tcaac10["MAT_THICK"] = tcaai03["MAT_THICK"].ToDecimal();
			tcaac10["SG_SIGN"] = tcaai03["SG_SIGN"].ToString();

			sqlstr = " select distinct mat_code from tcaac10"
				" where  1=1 "
				" and type_code = 'unit'"
				" and COST_CENTER=@cost_center"
				" and  dept_code=@dept_code "
				;
			cmd_inq_sub.SetCommandText(sqlstr);
			cmd_inq_sub.Parameters.Set("dept_code", dept_code);
			cmd_inq_sub.Parameters.Set("cost_center", tcaac10["COST_CENTER"]);
			cmd_inq_sub.ExecuteReader();
			while (cmd_inq_sub.Read())
			{
				tcaac10["MAT_CODE"] = cmd_inq_sub.GetString(1);

				if (tcaac10.QueryCount("DEPT_CODE,COST_CENTER,MAT_THICK,SG_SIGN,MAT_CODE") != 0)
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, @dept_code, @stats_period, @cost_center, @product_code, MAT_CODE, 0, sum(unit_cost1)*@wt,sum(unit_cost2)*@wt "
						" , @prod_date, @prod_shift_group, @prod_shift_no, @sub_backlog_code, @equ_no, @sg_sign, @mat_thick, @mat_width,@vfree1,@vfree2,@vfree3,@vfree4,@vfree5,'unit' "
						" from ("
						" select mat_code,decode(type_code,'GJ',unit_cost,0) unit_cost1,decode(type_code,'CJ',unit_cost,0) unit_cost2 "
						" from tcaac10 "
						" where 1=1 "
						" and sg_sign=@sg_sign"
						" and mat_thick=@mat_thick"
						" and mat_code = @mat_code"
						" and COST_CENTER=@cost_center"
						" and  dept_code=@dept_code )"
						" group by mat_code "
						;
				}
				else if (tcaac10.QueryCount("DEPT_CODE,COST_CENTER,MAT_THICK,MAT_CODE") != 0)
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, @dept_code, @stats_period, @cost_center, @product_code, MAT_CODE, 0, sum(unit_cost1)*@wt,sum(unit_cost2)*@wt "
						" , @prod_date, @prod_shift_group, @prod_shift_no, @sub_backlog_code, @equ_no, @sg_sign, @mat_thick, @mat_width,@vfree1,@vfree2,@vfree3,@vfree4,@vfree5,'unit' "
						" from ("
						" select mat_code,decode(type_code,'GJ',unit_cost,0) unit_cost1,decode(type_code,'CJ',unit_cost,0) unit_cost2 "
						" from tcaac10 "
						" where 1=1 "
						" and SG_SIGN=' '"
						" and mat_thick=@mat_thick"
						" and mat_code = @mat_code"
						" and COST_CENTER=@cost_center"
						" and  dept_code=@dept_code )"
						" group by mat_code "
						;
				}
				else if (tcaac10.QueryCount("DEPT_CODE,COST_CENTER,SG_SIGN,MAT_CODE") != 0)
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, @dept_code, @stats_period, @cost_center, @product_code, MAT_CODE, 0, sum(unit_cost1)*@wt,sum(unit_cost2)*@wt "
						" , @prod_date, @prod_shift_group, @prod_shift_no, @sub_backlog_code, @equ_no, @sg_sign, @mat_thick, @mat_width,@vfree1,@vfree2,@vfree3,@vfree4,@vfree5,'unit' "
						" from ("
						" select mat_code,decode(type_code,'GJ',unit_cost,0) unit_cost1,decode(type_code,'CJ',unit_cost,0) unit_cost2 "
						" from tcaac10 "
						" where 1=1 "
						" and SG_SIGN=@sg_sign"
						" and mat_thick=0"
						" and mat_code = @mat_code"
						" and COST_CENTER=@cost_center"
						" and  dept_code=@dept_code )"
						" group by mat_code "
						;
				}
				else
				{
					sqlstr = " insert into tcaai04(REC_CREATE_TIME, DEPT_CODE, STATS_PERIOD, COST_CENTER, PRODUCT_CODE, MAT_CODE, MAT_USE_1, MAT_COST_1,MAT_COST_2"
						" , prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width,vfree1,vfree2,vfree3,vfree4,vfree5,data_from )"
						" SELECT @datenow, @dept_code, @stats_period, @cost_center, @product_code, MAT_CODE, 0, sum(unit_cost1)*@wt,sum(unit_cost2)*@wt "
						" , @prod_date, @prod_shift_group, @prod_shift_no, @sub_backlog_code, @equ_no, @sg_sign, @mat_thick, @mat_width,@vfree1,@vfree2,@vfree3,@vfree4,@vfree5,'unit' "
						" from ("
						" select mat_code,decode(type_code,'GJ',unit_cost,0) unit_cost1,decode(type_code,'CJ',unit_cost,0) unit_cost2 "
						" from tcaac10 "
						" where 1=1 "
						" and SG_SIGN=' '"
						" and mat_thick=0"
						" and mat_code = @mat_code"
						" and COST_CENTER=@cost_center"
						" and  dept_code=@dept_code )"
						" group by mat_code "
						;
				}
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("mat_code", tcaac10["MAT_CODE"]);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("datenow", datenow);
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.Parameters.Set("wt", tcaai03["WT"]);
					cmd_inq_1.Parameters.Set("cost_center", tcaai03["COST_CENTER"]);
					cmd_inq_1.Parameters.Set("product_code", tcaai03["PRODUCT_CODE"]);
					cmd_inq_1.Parameters.Set("vfree1", tcaai03["VFREE1"]);
					cmd_inq_1.Parameters.Set("vfree2", tcaai03["VFREE2"]);
					cmd_inq_1.Parameters.Set("vfree3", tcaai03["VFREE3"]);
					cmd_inq_1.Parameters.Set("vfree4", tcaai03["VFREE4"]);
					cmd_inq_1.Parameters.Set("vfree5", tcaai03["VFREE5"]);
					cmd_inq_1.Parameters.Set("prod_date", tcaai03["PROD_DATE"]);
					cmd_inq_1.Parameters.Set("prod_shift_group", tcaai03["PROD_SHIFT_GROUP"]);
					cmd_inq_1.Parameters.Set("prod_shift_no", tcaai03["PROD_SHIFT_NO"]);
					cmd_inq_1.Parameters.Set("mat_thick", tcaai03["MAT_THICK"]);
					cmd_inq_1.Parameters.Set("mat_width", tcaai03["MAT_WIDTH"]);
					cmd_inq_1.Parameters.Set("equ_no", tcaai03["EQU_NO"]);
					cmd_inq_1.Parameters.Set("sg_sign", tcaai03["SG_SIGN"]);
					cmd_inq_1.Parameters.Set("sub_backlog_code", tcaai03["SUB_BACKLOG_CODE"]);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();

			}
			cmd_inq_sub.Close();
		}
		cmd_inq.Close();

			

			//更新金额
			sqlstr = " update tcaai03 t1 set (mat_cost_1,mat_cost_2) = (select sum(mat_cost_1),sum(mat_cost_2) from tcaai04 where 1=1 "
				" and COST_CENTER =@tcaai03.COST_CENTER and PRODUCT_CODE =@tcaai03.PRODUCT_CODE  "
				" and vfree1=@tcaai03.VFREE1 and vfree2=@tcaai03.VFREE2 and vfree3=@tcaai03.VFREE3 and vfree4=@tcaai03.VFREE4 and vfree5=@tcaai03.VFREE5"
				" and prod_date=@tcaai03.PROD_DATE and prod_shift_group=@tcaai03.PROD_SHIFT_GROUP  and prod_shift_no=@tcaai03.PROD_SHIFT_NO  and mat_thick=@tcaai03.MAT_THICK and mat_width=@tcaai03.MAT_WIDTH  and equ_no=@tcaai03.EQU_NO and sg_sign=@tcaai03.SG_SIGN and sub_backlog_code=@tcaai03.SUB_BACKLOG_CODE "
				")"
				" where 1=1"
				" and exists (select 1 from tcaai04 where 1=1 "
				" and COST_CENTER =@tcaai03.COST_CENTER and PRODUCT_CODE =@tcaai03.PRODUCT_CODE  "
				" and vfree1=@tcaai03.VFREE1 and vfree2=@tcaai03.VFREE2 and vfree3=@tcaai03.VFREE3 and vfree4=@tcaai03.VFREE4 and vfree5=@tcaai03.VFREE5"
				" and prod_date=@tcaai03.PROD_DATE and prod_shift_group=@tcaai03.PROD_SHIFT_GROUP  and prod_shift_no=@tcaai03.PROD_SHIFT_NO  and mat_thick=@tcaai03.MAT_THICK and mat_width=@tcaai03.MAT_WIDTH  and equ_no=@tcaai03.EQU_NO and sg_sign=@tcaai03.SG_SIGN and sub_backlog_code=@tcaai03.SUB_BACKLOG_CODE "
				" )"
				" and t1.COST_CENTER =@tcaai03.COST_CENTER and t1.PRODUCT_CODE =@tcaai03.PRODUCT_CODE  "
				" and t1.vfree1=@tcaai03.VFREE1 and t1.vfree2=@tcaai03.VFREE2 and t1.vfree3=@tcaai03.VFREE3 and t1.vfree4=@tcaai03.VFREE4 and t1.vfree5=@tcaai03.VFREE5"
				" and t1.prod_date=@tcaai03.PROD_DATE and t1.prod_shift_group=@tcaai03.PROD_SHIFT_GROUP  and t1.prod_shift_no=@tcaai03.PROD_SHIFT_NO  and t1.mat_thick=@tcaai03.MAT_THICK and t1.mat_width=@tcaai03.MAT_WIDTH  and t1.equ_no=@tcaai03.EQU_NO and t1.sg_sign=@tcaai03.SG_SIGN and t1.sub_backlog_code=@tcaai03.SUB_BACKLOG_CODE "
				" AND dept_code = @dept_code"
				" AND stats_period = @stats_period"
				;
			//Log::Trace("", "", "sqlstr = [{0}][{1}][{2}][{3}][{4}]", sqlstr, dept_code, stats_period, tcaai03.SUB_BACKLOG_CODE, tcaai03.PROD_DATE);
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.Parameters.Set("stats_period", stats_period);
			cmd_inq_1.Parameters.Set("tcaai03.WT", tcaai03["WT"]);
			cmd_inq_1.Parameters.Set("tcaai03.COST_CENTER", tcaai03["COST_CENTER"]);
			cmd_inq_1.Parameters.Set("tcaai03.PRODUCT_CODE", tcaai03["PRODUCT_CODE"]);
			cmd_inq_1.Parameters.Set("tcaai03.VFREE1", tcaai03["VFREE1"]);
			cmd_inq_1.Parameters.Set("tcaai03.VFREE2", tcaai03["VFREE2"]);
			cmd_inq_1.Parameters.Set("tcaai03.VFREE3", tcaai03["VFREE3"]);
			cmd_inq_1.Parameters.Set("tcaai03.VFREE4", tcaai03["VFREE4"]);
			cmd_inq_1.Parameters.Set("tcaai03.VFREE5", tcaai03["VFREE5"]);
			cmd_inq_1.Parameters.Set("tcaai03.PROD_DATE", tcaai03["PROD_DATE"]);
			cmd_inq_1.Parameters.Set("tcaai03.PROD_SHIFT_GROUP", tcaai03["PROD_SHIFT_GROUP"]);
			cmd_inq_1.Parameters.Set("tcaai03.PROD_SHIFT_NO", tcaai03["PROD_SHIFT_NO"]);
			cmd_inq_1.Parameters.Set("tcaai03.MAT_THICK", tcaai03["MAT_THICK"]);
			cmd_inq_1.Parameters.Set("tcaai03.MAT_WIDTH", tcaai03["MAT_WIDTH"]);
			cmd_inq_1.Parameters.Set("tcaai03.EQU_NO", tcaai03["EQU_NO"]);
			cmd_inq_1.Parameters.Set("tcaai03.SG_SIGN", tcaai03["SG_SIGN"]);
			cmd_inq_1.Parameters.Set("tcaai03.SUB_BACKLOG_CODE", tcaai03["SUB_BACKLOG_CODE"]);
			cmd_inq_1.ExecuteNonQuery();
			cmd_inq_1.Close();


		}
		cmd_inq.Close();

		//包装信息
		sqlstr = " update tcaai03 set UNIT_PRICE = round(mat_cost_1/WT,6)"
			" where 1=1"
			" and wt!=0"
			" AND dept_code = @dept_code"
			" AND stats_period = @stats_period"
			;
		Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();		

		//更新物料名称
		sqlstr = "update tcaai04 t1 set mat_name = (select mat_name from tcaac11 t2 where t1.mat_code = t2.mat_code)"
			" where 1=1"
			" and exists (select 1 from tcaac11 t2 where t1.mat_code = t2.mat_code)"
			" AND dept_code = @dept_code"
			" AND stats_period = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
