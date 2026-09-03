/*=========================================================================
//程序名称:     f_caai_01
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:
//修改日期:
//函数说明：针对消耗的进行价格更新
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件
BM2_FUNCTION_EXPORT

int f_caai_updprice(CString stats_period, CString dept_code, CString type, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;
	CDecimal price_gj = 0;
	CDecimal price_cj = 0;
	CDecimal price_yj = 0;
	CDecimal all_wt = 0;
	CString shift_no = "";
	CString shift_group = "";

	CString sqlstr = "";
	CString sqlstr1 = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);
	CDbCommand cmd_inq_a(conn);
	
	try
	{	//更新采购价
			sqlstr = " select distinct mat_code from tcaai06 "
				" WHERE 1=1"
				" and wt<>0 "
				" AND dept_code = @dept_code"
				" AND account_period = @stats_period"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				sqlstr = "SELECT PRICE_TERMS,PRICE_UNIT FROM TCAAC12 "   
					" WHERE 1=1 "
					" AND (PRICE_TERMS,VALID_TIME_START) in (select PRICE_TERMS,max(VALID_TIME_START) from tcaac12 where MAT_CODE = @mat_code and VALID_TIME_START<=@stats_period group by PRICE_TERMS)"
					" AND MAT_CODE = @mat_code "
					" and VALID_TIME_START<=@stats_period"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.ExecuteReader();
				price_yj = 0;
				price_gj = 0;
				price_cj = 0;
				while (cmd_inq_1.Read())
				{
					if (cmd_inq_1.GetString(1) == "YJ")
					{
						price_yj = cmd_inq_1.GetDecimal(2);
					}
					if (cmd_inq_1.GetString(1) == "GJ")
					{
						price_gj = cmd_inq_1.GetDecimal(2);
					}
					if (cmd_inq_1.GetString(1) == "CJ")
					{
						price_cj = cmd_inq_1.GetDecimal(2);
					}
				}
				cmd_inq_1.Close();
				
				sqlstr = " update tcaai06 set cost_gj=round(wt*@price_gj,6),cost_yj=round(wt*@price_yj,6),cost_cj=round(wt*@price_cj,6) "
					" ,price_unit_gj = @price_gj,price_unit_yj = @price_yj,price_unit_cj = @price_cj"
					" WHERE 1=1"
					" and mat_code=@mat_code"
					" and wt<>0 "
					" AND dept_code= @dept_code"
					" AND account_period = @stats_period"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("price_gj", price_gj);
				cmd_inq_1.Parameters.Set("price_yj", price_yj);
				cmd_inq_1.Parameters.Set("price_cj", price_cj);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();

			}
			cmd_inq.Close();

			//对应更新金额
			sqlstr = " update tcaai05 t1 set (cost_hj,cost_gj,cost_yj,cost_cj) "
				" = (select sum(cost_hj),sum(cost_gj),sum(cost_yj),sum(cost_cj) from tcaai06 t2 where t1.sub_backlog_code=t2.sub_backlog_code and t1.product_CODE = t2.product_CODE and t1.equ_no = t2.equ_no and t1.sg_sign = t2.sg_sign and t1.mat_thick = t2.mat_thick and t1.mat_width = t2.mat_width and t1.vfree1 = t2.vfree1 and t1.vfree2=t2.vfree2 and t1.vfree3=t2.vfree3 and t1.vfree4=t2.vfree4 and t1.vfree5 = t2.vfree5 and t2.dept_code =@dept_code and t2.account_period = @stats_period  )"
				
				" WHERE 1=1"
				" and exists (select 1 from tcaai06 t2 where t1.sub_backlog_code=t2.sub_backlog_code and t1.product_CODE = t2.product_CODE and t1.equ_no = t2.equ_no and t1.sg_sign = t2.sg_sign and t1.mat_thick = t2.mat_thick and t1.mat_width = t2.mat_width and t1.vfree1 = t2.vfree1 and t1.vfree2=t2.vfree2 and t1.vfree3=t2.vfree3 and t1.vfree4=t2.vfree4 and t1.vfree5 = t2.vfree5 and t2.dept_code =@dept_code and t2.account_period = @stats_period  )"
				" AND dept_code= @dept_code"
				" AND account_period = @stats_period"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			//更新单价
			sqlstr = " update tcaai05 set PRICE_UNIT = round(cost_hj/WT,6),PRICE_UNIT_GJ = round(cost_gj/WT,6),PRICE_UNIT_cj = round(cost_cj/WT,6),PRICE_UNIT_yj = round(cost_yj/WT,6)"
				" where 1=1"
				" AND dept_code = @dept_code"
				" AND account_period = @stats_period"
				;
			Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
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
